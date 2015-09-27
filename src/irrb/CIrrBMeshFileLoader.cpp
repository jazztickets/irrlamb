//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#define _IRR_COMPILE_WITH_IRRB_MESH_LOADER_
#ifdef _IRR_COMPILE_WITH_IRRB_MESH_LOADER_
#include "CIrrBMeshWriter.h"
#include "CIrrBMeshFileLoader.h"
#include "IXMLReader.h"
#include "SAnimatedMesh.h"
#include "fast_atof.h"
#include "IReadFile.h"
#include "IAttributes.h"
#include "IMeshSceneNode.h"
#include "SMeshBufferLightMap.h"
#include "CDynamicMeshBuffer.h"

namespace irr
{
namespace scene
{


//! Constructor
CIrrBMeshFileLoader::CIrrBMeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs)
	: SceneManager(smgr), FileSystem(fs)
{
	Driver = smgr->getVideoDriver();
}


//! destructor
CIrrBMeshFileLoader::~CIrrBMeshFileLoader()
{
}


//! Returns true if the file maybe is able to be loaded by this class.
/** This decision should be based only on the file extension (e.g. ".cob") */
bool CIrrBMeshFileLoader::isALoadableFileExtension(const io::path& filename) const
{
	return core::hasFileExtension ( filename, "irrbmesh");
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CIrrBMeshFileLoader::createMesh(io::IReadFile* file)
{
	IAnimatedMesh* mesh = readMesh(file);

	return mesh;
}


//! reads a mesh section and creates a mesh from it
IAnimatedMesh* CIrrBMeshFileLoader::readMesh(io::IReadFile* reader)
{
	struct irr::scene::IrrbHeader ih;
	u32    sigCheck = MAKE_IRR_ID('i','r','r','b');
	//u8     vmaj, vmin;
	u32    sysinfo;


	Reader = reader;

	Reader->seek(0);
	if(Reader->read(&ih,sizeof(ih)) != sizeof(ih))
		return 0;

	// sanity checks

	if(ih.hSigCheck != sigCheck)
		return 0;

	if(ih.hMeshCount < 1)
		return 0;

	// endianess & int bit size, no conversion for now so abort.
	sysinfo = (get_endianess() << 16) | (sizeof(int) * 8);
	if(ih.hFlags != sysinfo)
		return 0;

	//vmaj = (ih.hVersion & 0xFF00) >> 8;
	//vmin = ih.hVersion & 0xFF;

	SAnimatedMesh* animatedmesh = new SAnimatedMesh();
	core::aabbox3df mbb;
	mbb.reset(0.f,0.f,0.f);

	Materials.clear();

	for(u32 i=0; i<ih.hMeshCount; i++)
	{
		SMesh* mesh=0;
		mesh = _readMesh(i);
		if(mesh)
		{
			mbb.addInternalBox(mesh->getBoundingBox());
			animatedmesh->addMesh(mesh);
			mesh->drop();
		}
	}

	animatedmesh->setBoundingBox(mbb);

	return animatedmesh;
}

u32 CIrrBMeshFileLoader::readChunk(struct IrrbChunkInfo& chunk)
{
	return Reader->read(&chunk, sizeof(chunk));
}

irr::core::stringc CIrrBMeshFileLoader::readStringChunk()
{
	c8  buf[256];
	irr::core::stringc result="";

	struct IrrbChunkInfo chunk;
	Reader->read(&chunk, sizeof(chunk));
	if(chunk.iId != CID_STRING)
		return result;

	memset(buf,0,sizeof(buf));
	Reader->read(buf, chunk.iSize);
	result = buf;
	return result;

}

SMesh* CIrrBMeshFileLoader::_readMesh(u32 index)
{
	u32 idx=0;
	struct IrrbChunkInfo ci;
	struct IrrbMeshInfo  mi;


	// position to the correct mesh
	Reader->seek(sizeof(struct IrrbHeader));
	readChunk(ci);

	while(idx < index)
	{
		u32 cpos = Reader->getPos();
		Reader->seek(cpos+ci.iSize);
		readChunk(ci);
		++idx;
	}

	if(ci.iId != CID_MESH)
		return 0;

	Reader->read(&mi, sizeof(mi));
	if(mi.iMeshBufferCount == 0)
		return 0;

	//
	// read vertex & index data
	//

	u32 vbufsize,ibufsize;
	vbufsize = sizeof(struct IrrbVertex) * mi.iVertexCount;
	ibufsize = sizeof(u32) * mi.iIndexCount;

	VBuffer = (struct IrrbVertex*)malloc(vbufsize);
	IBuffer = (u32 *)malloc(ibufsize);

	Reader->read(VBuffer,vbufsize);
	Reader->read(IBuffer,ibufsize);

	//
	// read & create materials
	//
	u32 matBufSize = sizeof(struct IrrbMaterial);
	u32 layBufSize = sizeof(struct IrrbMaterialLayer)*4;
	Material = (struct IrrbMaterial*) malloc(matBufSize);
	Layer = (struct IrrbMaterialLayer*) malloc(layBufSize);
	for(u32 i=0; i<mi.iMaterialCount; i++)
	{
		Reader->read(Material,matBufSize);
		irr::video::SMaterial material;
		setMaterial(material,*Material);

		for(u32 j=0; j<Material->mLayerCount; j++)
		{
			irr::core::stringc textureName = readStringChunk();
			Reader->read(Layer,sizeof(struct IrrbMaterialLayer));
			setMaterialLayer(material, j, textureName, *Layer);
		}

		Materials.push_back(material);
	}
	free(Material);
	free(Layer);

	//
	// read meshbuffer data
	//
	readChunk(ci);
	if(ci.iId != CID_MESHBUF)
	{
		free(VBuffer);
		free(IBuffer);
		return 0;
	}

	SMesh* mesh = new SMesh();

	u32 mbiSize = mi.iMeshBufferCount * sizeof(struct IrrbMeshBufInfo);

	MBuffer = (IrrbMeshBufInfo*) malloc(mbiSize);
	Reader->read(MBuffer,mbiSize);

	for(idx=0; idx<mi.iMeshBufferCount; idx++)
	{
		IMeshBuffer* buffer = createMeshBuffer(idx);
		if(buffer)
		{
			mesh->addMeshBuffer(buffer);
			buffer->drop();
		}
	}

	free(MBuffer);
	free(VBuffer);
	free(IBuffer);


	//
	// todo add bounding box to irrbmesh format...
	//
	core::aabbox3df mbb(mi.ibbMin.x,mi.ibbMin.y,mi.ibbMin.z,
		mi.ibbMax.x,mi.ibbMax.y,mi.ibbMax.z);
	mesh->setBoundingBox(mbb);

	return mesh;
}

void CIrrBMeshFileLoader::setMaterialLayer(video::SMaterial& material, u8 layerNumber, irr::core::stringc mTexture, struct IrrbMaterialLayer& layer)
{

	video::IImage* img=0;
	video::ITexture* tex=0;

	if(layerNumber > 3)
		return;

	tex = Driver->findTexture(mTexture);
	if(!tex)
	{
		img = Driver->createImageFromFile(mTexture);
		if(img)
		{
			tex = Driver->addTexture(mTexture,img);
			img->drop();
		}
	}
	if(tex)
	{
		material.TextureLayer[layerNumber].Texture = tex;
		material.TextureLayer[layerNumber].BilinearFilter = layer.mBilinearFilter;
		material.TextureLayer[layerNumber].TrilinearFilter = layer.mTrilinearFilter;
		material.TextureLayer[layerNumber].AnisotropicFilter = layer.mAnisotropicFilter;
		material.TextureLayer[layerNumber].LODBias = layer.mLODBias;
		material.TextureLayer[layerNumber].TextureWrapU = (irr::video::E_TEXTURE_CLAMP)layer.mTextureWrapU;
		material.TextureLayer[layerNumber].TextureWrapV = (irr::video::E_TEXTURE_CLAMP)layer.mTextureWrapV;
		irr::core::matrix4 mat4;
		memcpy(mat4.pointer(),&layer.mMatrix,sizeof(u16)*16);
		material.TextureLayer[layerNumber].setTextureMatrix(mat4);
	}
}

void CIrrBMeshFileLoader::setMaterial(video::SMaterial& material, struct IrrbMaterial& mat)
{
	material.MaterialType = (irr::video::E_MATERIAL_TYPE)mat.mType;
	material.AmbientColor.color = mat.mAmbient;
	material.DiffuseColor.color= mat.mDiffuse;
	material.EmissiveColor.color = mat.mEmissive;
	material.SpecularColor.color = mat.mSpecular;
	material.Shininess = mat.mShininess;
	material.MaterialTypeParam = mat.mParm1;
	material.MaterialTypeParam2 = mat.mParm2;
	material.Wireframe = mat.mWireframe;
	material.GouraudShading = mat.mGrouraudShading;
	material.Lighting = mat.mLighting;
	material.ZWriteEnable = mat.mZWriteEnabled;
	material.ZBuffer = mat.mZBuffer;
	material.BackfaceCulling = mat.mBackfaceCulling;
	material.FogEnable = mat.mFogEnable;
	material.NormalizeNormals = mat.mNormalizeNormals;
	material.AntiAliasing = mat.mAntiAliasing;
	material.ColorMask = mat.mColorMask;
	material.ColorMaterial = mat.mColorMaterial;
	material.UseMipMaps = mat.mUseMipMaps;
	material.PolygonOffsetDirection = (irr::video::E_POLYGON_OFFSET)mat.mPolygonOffsetDirection;
	material.PolygonOffsetFactor = mat.mPolygonOffsetFactor;
	material.BlendOperation = (irr::video::E_BLEND_OPERATION)mat.mBlendOperation;
}


IMeshBuffer* CIrrBMeshFileLoader::createMeshBuffer(u32 idx)
{
	CDynamicMeshBuffer*     buffer = 0;
	struct IrrbVertex       *pivb;
	u32                     *pindices;
	video::E_INDEX_TYPE     iType=video::EIT_16BIT;

	struct IrrbMeshBufInfo& mbi=MBuffer[idx];
	pivb = &VBuffer[mbi.iVertStart];
	pindices = &IBuffer[mbi.iIndexStart];
	if(mbi.iIndexCount > 65536)
		iType = video::EIT_32BIT;

	buffer = new CDynamicMeshBuffer((video::E_VERTEX_TYPE)mbi.iVertexType, iType);
	scene::IVertexBuffer& Vertices = buffer->getVertexBuffer();
	buffer->Material = Materials[mbi.iMaterialIndex];

	for(idx=0; idx<mbi.iVertCount; idx++)
	{

		video::S3DVertex vtx0;
		video::S3DVertex2TCoords vtx1;
		video::S3DVertexTangents vtx2;

		video::S3DVertex* vtx=0;


		if(mbi.iVertexType == irr::video::EVT_2TCOORDS)
			vtx = &vtx1;
		else if(mbi.iVertexType == irr::video::EVT_TANGENTS)
			vtx = &vtx2;
		else vtx = &vtx0;

		// set common data
		vtx->Pos.X = pivb->vPos.x;
		vtx->Pos.Y = pivb->vPos.y;
		vtx->Pos.Z = pivb->vPos.z;

		vtx->Normal.X = pivb->vNormal.x;
		vtx->Normal.Y = pivb->vNormal.y;
		vtx->Normal.Z = pivb->vNormal.z;

		vtx->Color = pivb->vColor;

		vtx->TCoords.X = pivb->vUV1.x;
		vtx->TCoords.Y = pivb->vUV1.y;

		if(mbi.iVertexType == irr::video::EVT_2TCOORDS)
		{
			vtx1.TCoords2.X = pivb->vUV2.x;
			vtx1.TCoords2.Y = pivb->vUV2.y;
			Vertices.push_back(vtx1);
		}
		else if(mbi.iVertexType == irr::video::EVT_TANGENTS)
		{
			vtx2.Tangent.X = pivb->vTangent.x;
			vtx2.Tangent.Y = pivb->vTangent.y;
			vtx2.Tangent.Z = pivb->vTangent.z;

			vtx2.Binormal.X = pivb->vBiNormal.x;
			vtx2.Binormal.Y = pivb->vBiNormal.y;
			vtx2.Binormal.Z = pivb->vBiNormal.z;
			Vertices.push_back(vtx2);
		}
		else
		{
			Vertices.push_back(vtx0);

		}
		++pivb;
	}

	scene::IIndexBuffer& Indices = buffer->getIndexBuffer();

	for(idx=0; idx<mbi.iIndexCount; idx++)
	{
		Indices.push_back(*pindices);
		++pindices;
	}

	core::aabbox3df mbb(mbi.ibbMin.x,mbi.ibbMin.y,mbi.ibbMin.z,
		mbi.ibbMax.x,mbi.ibbMax.y,mbi.ibbMax.z);
	buffer->setBoundingBox(mbb);

	return buffer;
}

} // end namespace scene
} // end namespace irr

#endif // _IRR_COMPILE_WITH_IRRB_MESH_LOADER_
