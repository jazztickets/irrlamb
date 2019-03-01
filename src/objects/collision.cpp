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
#include <objects/collision.h>
#include <physics.h>
#include <globals.h>
#include <objects/template.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <fstream>

static bool CustomMaterialCallback(btManifoldPoint &ManifoldPoint, const btCollisionObjectWrapper *Object0, int PartID0, int Index0, const btCollisionObjectWrapper *Object1, int PartID1, int Index1) {

	//if(Object0->getCollisionShape()->getShapeType()TRIANGLE_SHAPE_PROXYTYPE
	//printf("before %f\n", ManifoldPoint.m_normalWorldOnB[1]);
	//float Before = ManifoldPoint.m_normalWorldOnB[1];
	if(1) {
		//printf("%d %d %d %d %d %d\n", Object0->getCollisionShape()->getShapeType(), Object1->getCollisionShape()->getShapeType(), PartID0, PartID1, Index0, Index1);
		btAdjustInternalEdgeContacts(ManifoldPoint, Object1, Object0, PartID1, Index1);
	}
	//float After = ManifoldPoint.m_normalWorldOnB[1];
	//if(Before != After) printf("before %f after %f\n", Before, After);

	return false;
}


// Constructor
_Collision::_Collision(const _ObjectSpawn &Object)
:	_Object(Object.Template),
	TriangleIndexVertexArray(nullptr),
	TriangleInfoMap(nullptr),
	VertexList(nullptr),
	FaceList(nullptr) {

	gContactAddedCallback = CustomMaterialCallback;

	// Load collision mesh file
	std::ifstream MeshFile(Object.Template->CollisionFile.c_str(), std::ios::binary);
	if(MeshFile) {

		// Read header
		int VertCount, FaceCount;
		MeshFile.read((char *)&VertCount, sizeof(VertCount));
		MeshFile.read((char *)&FaceCount, sizeof(FaceCount));

		// Allocate memory for lists
		VertexList = new float[VertCount * 3];
		FaceList = new int[FaceCount * 3];

		// Read vertices
		int VertexIndex = 0;
		for(int i = 0; i < VertCount; i++) {
			float Value;
			MeshFile.read((char *)&Value, sizeof(Value));
			VertexList[VertexIndex++] = Value;

			MeshFile.read((char *)&Value, sizeof(Value));
			VertexList[VertexIndex++] = Value;

			MeshFile.read((char *)&Value, sizeof(Value));
			VertexList[VertexIndex++] = -Value;
		}

		// Read faces
		int FaceIndex = 0;
		for(int i = 0; i < FaceCount; i++) {
			int Value;

			MeshFile.read((char *)&Value, sizeof(Value));
			FaceList[FaceIndex+2] = Value;

			MeshFile.read((char *)&Value, sizeof(Value));
			FaceList[FaceIndex+1] = Value;

			MeshFile.read((char *)&Value, sizeof(Value));
			FaceList[FaceIndex+0] = Value;

			FaceIndex += 3;
		}

		// Create triangle array
		TriangleIndexVertexArray = new btTriangleIndexVertexArray(FaceCount, FaceList, 3 * sizeof(int), VertCount * 3, VertexList, 3 * sizeof(float));

		// Create bvh shape
		btBvhTriangleMeshShape *Shape = new btBvhTriangleMeshShape(TriangleIndexVertexArray, true);
		TriangleInfoMap = new btTriangleInfoMap();
		btGenerateInternalEdgeInfo(Shape, TriangleInfoMap);

		// Create physics body
		CreateRigidBody(Object, Shape);
		SetProperties(Object);

		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

		MeshFile.close();
	}
}

// Destructor
_Collision::~_Collision() {

	delete TriangleInfoMap;
	delete TriangleIndexVertexArray;
	delete[] VertexList;
	delete[] FaceList;
}
