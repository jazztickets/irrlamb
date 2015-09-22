/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2015  Alan Witkowski
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
#include <engine/globals.h>
#include <engine/physics.h>
#include <engine/config.h>
#include <objects/template.h>
#include <ITerrainSceneNode.h>
#include <CDynamicMeshBuffer.h>
#include <ISceneManager.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

using namespace irr;

// Constructor
_Terrain::_Terrain(const SpawnStruct &Object)
:	_Object(),
	CollisionMesh(NULL) {
	TemplateStruct *Template = Object.Template;

	// Check for mesh file
	if(Template->Mesh != "") {
		std::string HeightMap = "textures/" + Template->Mesh;
		scene::ITerrainSceneNode *Terrain = irrScene->addTerrainSceneNode(
			HeightMap.c_str(),
			0,
			-1,
			core::vector3df(0.0f, 0.0f, 0.0f),
			core::vector3df(0.0f, 0.0f, 0.0f),
			core::vector3df(1.0f, 1.0f, 1.0f),
			video::SColor(255, 255, 255, 255),
			5,
			scene::ETPS_17,
			Template->Smooth);

		// Set node transform
		Terrain->setPosition(core::vector3df(Object.Position[0], Object.Position[1], Object.Position[2]));
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

		Node = Terrain;
		if(Physics.IsEnabled()) {
			CollisionMesh = new btTriangleMesh();
			scene::CDynamicMeshBuffer MeshBuffer(video::EVT_STANDARD, video::EIT_32BIT);
			Terrain->getMeshBufferForLOD(MeshBuffer, 0);
			u16 *Indices = MeshBuffer.getIndices();

			video::S3DVertex *Vertices = (video::S3DVertex *)MeshBuffer.getVertices();
			btVector3 TriangleVertices[3];
			for(u32 i = 0; i < MeshBuffer.getIndexCount(); i += 3) {

				TriangleVertices[0] = btVector3(Vertices[Indices[i]].Pos.X * Template->Shape[0],
												Vertices[Indices[i]].Pos.Y * Template->Shape[1],
												Vertices[Indices[i]].Pos.Z * Template->Shape[2]);
				TriangleVertices[1] = btVector3(Vertices[Indices[i+1]].Pos.X * Template->Shape[0],
												Vertices[Indices[i+1]].Pos.Y * Template->Shape[1],
												Vertices[Indices[i+1]].Pos.Z * Template->Shape[2]);
				TriangleVertices[2] = btVector3(Vertices[Indices[i+2]].Pos.X * Template->Shape[0],
												Vertices[Indices[i+2]].Pos.Y * Template->Shape[1],
												Vertices[Indices[i+2]].Pos.Z * Template->Shape[2]);

				CollisionMesh->addTriangle(TriangleVertices[0], TriangleVertices[1], TriangleVertices[2]);
			}

			btBvhTriangleMeshShape *Shape = new btBvhTriangleMeshShape(CollisionMesh, true);

			// Create physics body
			CreateRigidBody(Object, Shape);
			SetProperties(Object);

			RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}
	}
}

// Destructor
_Terrain::~_Terrain() {
	delete CollisionMesh;
}
