//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#ifndef __C_IRRB_MESH_FILE_LOADER_H_INCLUDED__
#define __C_IRRB_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "IVideoDriver.h"
#include "irrString.h"
#include "SMesh.h"
#include "SMeshBuffer.h"
#include "ISceneManager.h"

namespace irr
{
namespace scene
{


//! Meshloader capable of loading .irrbmesh meshes, the Irrlicht Engine binary mesh format for static meshes
class CIrrBMeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CIrrBMeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs);

	//! destructor
	virtual ~CIrrBMeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".cob")
	virtual bool isALoadableFileExtension(const io::path& filename) const;


	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IReferenceCounted::drop() for more information.
	virtual IAnimatedMesh* createMesh(io::IReadFile* file);

private:

	//! reads a mesh sections and creates a mesh from it
	IAnimatedMesh* readMesh(io::IReadFile* reader);

	u32 readChunk(struct IrrbChunkInfo& chunk);
	irr::core::stringc readStringChunk();

	SMesh* _readMesh(u32 index);
	IMeshBuffer* createMeshBuffer(u32 idx);
	void setMaterial(video::SMaterial& material, struct IrrbMaterial& mat);
	void setMaterialLayer(video::SMaterial& material, u8 layerNumber, irr::core::stringc mTexture, struct IrrbMaterialLayer& layer);

	// member variables
	u32* IBuffer;
	struct IrrbVertex* VBuffer;
	struct IrrbMeshBufInfo* MBuffer;
	struct IrrbMaterial* Material;
	struct IrrbMaterialLayer* Layer;
	core::array<video::SMaterial> Materials;

	video::IVideoDriver* Driver;
	scene::ISceneManager* SceneManager;
	io::IFileSystem* FileSystem;
	io::IReadFile* Reader;
};


} // end namespace scene
} // end namespace irr

#endif
