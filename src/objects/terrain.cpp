/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2019  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <objects/terrain.h>
#include <globals.h>
#include <physics.h>
#include <config.h>
#include <level.h>
#include <save.h>
#include <objects/template.h>
#include <ITerrainSceneNode.h>
#include <CDynamicMeshBuffer.h>
#include <ISceneManager.h>
#include <fstream>

using namespace irr;

// Constructor
_Terrain::_Terrain(const _ObjectSpawn &Object) :
	_Object(Object.Template),
	TriMeshData(nullptr),
	VertexList(nullptr),
	FaceList(nullptr) {

	// Check for mesh file
	if(Template->Mesh != "") {
		std::string HeightMap = "textures/" + Template->Mesh;
		scene::ITerrainSceneNode *Terrain = irrScene->addTerrainSceneNode(
			HeightMap.c_str(),
			0,
			-1,
			core::vector3df(Object.Position[0], Object.Position[1], Object.Position[2]),
			core::vector3df(Object.Rotation[0], Object.Rotation[1], Object.Rotation[2]),
			core::vector3df(Template->Shape[0], Template->Shape[1], Template->Shape[2]),
			video::SColor(255, 255, 255, 255),
			5,
			scene::ETPS_17,
			Template->Smooth
		);

		Node = Terrain;
		//Node->setMaterialFlag(video::EMF_WIREFRAME, true);

		// Get original rotation pivot used to transform terrain
		core::vector3df OriginalRotationPivot = Terrain->getTerrainCenter();

		// Force normal recalculation
		Terrain->setScale(core::vector3df(Template->Shape[0], Template->Shape[1], Template->Shape[2]));

		// Set textures
		for(int i = 0; i < 2; i++) {
			if(Template->Textures[i] != "")
				Terrain->setMaterialTexture(i, irrDriver->getTexture(Template->Textures[i].c_str()));
		}
		Terrain->scaleTexture(Template->TextureScale[0], Template->TextureScale[1]);

		//Terrain->setMaterialType(video::EMT_DETAIL_MAP);
		if(Template->CustomMaterial != -1)
			Terrain->setMaterialType((video::E_MATERIAL_TYPE)Template->CustomMaterial);

		if(Physics.IsEnabled()) {

			// Get vertex data
			scene::CDynamicMeshBuffer MeshBuffer(video::EVT_STANDARD, video::EIT_32BIT);
			Terrain->getMeshBufferForLOD(MeshBuffer, 0);
			uint16_t *Indices = MeshBuffer.getIndices();

			// Allocate memory for lists
			int VertexCount = MeshBuffer.getIndexCount() * 3;
			int IndexCount = MeshBuffer.getIndexCount();
			VertexList = new float[VertexCount];
			FaceList = new dTriIndex[IndexCount];

			// Transform vertices
			core::matrix4 RotationTransform;
			RotationTransform.setRotationDegrees(Terrain->getRotation());
			video::S3DVertex *Vertices = (video::S3DVertex *)MeshBuffer.getVertices();

			// Get face and vertex list
			int VertexIndex = 0;
			for(int i = 0; i < IndexCount; i += 3) {
				FaceList[i+0] = i+0;
				FaceList[i+1] = i+1;
				FaceList[i+2] = i+2;
				for(int j = 0; j < 3; j++) {

					// Apply terrain transform
					core::vector3df Vertex = Vertices[Indices[i+j]].Pos * Terrain->getScale() + Terrain->getPosition();
					Vertex -= OriginalRotationPivot;
					RotationTransform.inverseRotateVect(Vertex);
					Vertex += OriginalRotationPivot;

					// Set triangle
					VertexList[VertexIndex++] = Vertex.X;
					VertexList[VertexIndex++] = Vertex.Y;
					VertexList[VertexIndex++] = Vertex.Z;
				}
			}

			// Create trimesh
			TriMeshData = dGeomTriMeshDataCreate();
			dGeomTriMeshDataBuildSingle1(TriMeshData, VertexList, 3 * sizeof(float), VertexCount / 3, FaceList, IndexCount, 3 * sizeof(dTriIndex), nullptr);
			Geometry = dCreateTriMesh(Physics.GetSpace(), TriMeshData, 0, 0, 0);
		}

		SetProperties(Object, false);
	}
}

// Destructor
_Terrain::~_Terrain() {
	dGeomTriMeshDataDestroy(TriMeshData);
	delete[] VertexList;
	delete[] FaceList;
}

// Get path to terrain cache file
std::string _Terrain::GetCachePath(const std::string &ObjectName) {
	return Save.CachePath + Level.LevelName + "_" + std::to_string(Level.LevelVersion) + "_" + ObjectName + ".cache";
}
