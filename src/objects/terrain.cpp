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
#include <objects/template.h>
#include <ITerrainSceneNode.h>
#include <CDynamicMeshBuffer.h>
#include <ISceneManager.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

using namespace irr;

// Constructor
_Terrain::_Terrain(const _ObjectSpawn &Object)
:	_Object(),
	CollisionMesh(nullptr) {
	_Template *Template = Object.Template;

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
			Template->Smooth);

		Node = Terrain;

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

			// Transform vertices
			core::matrix4 RotationTransform;
			RotationTransform.setRotationDegrees(Terrain->getRotation());
			video::S3DVertex *Vertices = (video::S3DVertex *)MeshBuffer.getVertices();

			// Create bullet mesh
			btVector3 TriangleVertices[3];
			CollisionMesh = new btTriangleMesh();
			for(uint32_t i = 0; i < MeshBuffer.getIndexCount(); i += 3) {
				for(int j = 0; j < 3; j++) {

					// Apply terrain transform
					core::vector3df Vertex = Vertices[Indices[i+j]].Pos * Terrain->getScale() + Terrain->getPosition();
					Vertex -= OriginalRotationPivot;
					RotationTransform.inverseRotateVect(Vertex);
					Vertex += OriginalRotationPivot;

					// Set triangle
					TriangleVertices[j] = btVector3(Vertex.X, Vertex.Y, Vertex.Z);
				}

				// Add triangle
				CollisionMesh->addTriangle(TriangleVertices[0], TriangleVertices[1], TriangleVertices[2]);
			}

			// Create bvh mesh
			btBvhTriangleMeshShape *Shape = new btBvhTriangleMeshShape(CollisionMesh, true);

			// Create physics body
			CreateRigidBody(Object, Shape, false);

			// Set flags
			RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}

		SetProperties(Object, false);
	}
}

// Destructor
_Terrain::~_Terrain() {
	delete CollisionMesh;
}
