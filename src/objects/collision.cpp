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
#include <fstream>
#include <ode/collision.h>

// Constructor
_Collision::_Collision(const _ObjectSpawn &Object) :
	_Object(Object.Template),
	TriMeshData(nullptr),
	VertexList(nullptr),
	FaceList(nullptr) {

	// Load collision mesh file
	std::ifstream MeshFile(Object.Template->CollisionFile.c_str(), std::ios::binary);
	if(MeshFile) {

		// Read header
		int VertexCount, FaceCount;
		MeshFile.read((char *)&VertexCount, sizeof(VertexCount));
		MeshFile.read((char *)&FaceCount, sizeof(FaceCount));

		// Allocate memory for lists
		VertexList = new float[VertexCount * 3];
		FaceList = new dTriIndex[FaceCount * 3];

		// Read vertices
		int VertexIndex = 0;
		for(int i = 0; i < VertexCount; i++) {
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

		// Close file
		MeshFile.close();

		// Create trimesh
		TriMeshData = dGeomTriMeshDataCreate();
		dGeomTriMeshDataBuildSingle1(TriMeshData, VertexList, 3 * sizeof(float), VertexCount, FaceList, FaceIndex, 3 * sizeof(dTriIndex), nullptr);
		Geometry = dCreateTriMesh(Physics.GetSpace(), TriMeshData, 0, 0, 0);
	}

	SetProperties(Object, false);
}

// Destructor
_Collision::~_Collision() {

	dGeomTriMeshDataDestroy(TriMeshData);
	delete[] VertexList;
	delete[] FaceList;
}
