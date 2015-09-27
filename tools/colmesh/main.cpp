/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2013  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace std;

// Structures
struct VertexStruct {
	float Data[3];
};

struct FaceStruct {
	int Data[3];
};

// Globals
static vector<VertexStruct> Vertices;
static vector<FaceStruct> Faces;

// Functions
bool ReadObjFile(const char *Filename);
bool WriteColFile(const char *Filename);

int main(int ArgumentCount, char **Arguments) {

	// Parse arguments
	if(ArgumentCount != 2) {
		printf("Needs 1 argument: .obj file\n");
		return EXIT_FAILURE;
	}

	// Parse file
	string File = Arguments[1];
	size_t Extension = File.rfind(".obj");
	if(Extension == string::npos) {
		printf("Bad argument: %s\n", Arguments[1]);
		return EXIT_FAILURE;
	}

	// Get filenames
	string BaseName = File.substr(0, Extension);
	string ObjFilename = BaseName + string(".obj");
	string ColFilename = BaseName + string(".col");

	// Read file
	if(!ReadObjFile(ObjFilename.c_str())) {
		return EXIT_FAILURE;
	}

	// Write file
	if(!WriteColFile(ColFilename.c_str())) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// Read an obj file and populate the vertices/faces list
bool ReadObjFile(const char *Filename) {

	// Open file
	ifstream InputFile(Filename);
	if(!InputFile.is_open()) {
		printf("Error opening %s for reading\n", Filename);

		return false;
	}

	// Read file
	char Buffer[256];
	bool HasTexture = false;
	while(!InputFile.eof()) {

		InputFile.getline(Buffer, 255);
		if(Buffer[0] == 'v') {
			if(Buffer[1] == ' ') {
				VertexStruct Vertex;

				sscanf(Buffer, "v %f %f %f", &Vertex.Data[0], &Vertex.Data[1], &Vertex.Data[2]);
				Vertices.push_back(Vertex);
			}
			else if(Buffer[1] == 't') {
				HasTexture = true;
			}
		}
		else if(Buffer[0] == 'f' && Buffer[1] == ' ') {
			FaceStruct Face;
			int Dummy;

			if(HasTexture)
				sscanf(Buffer, "f %d/%d %d/%d %d/%d", &Face.Data[0], &Dummy, &Face.Data[1], &Dummy, &Face.Data[2], &Dummy);
			else
				sscanf(Buffer, "f %d %d %d", &Face.Data[0], &Face.Data[1], &Face.Data[2]);

			Face.Data[0]--;
			Face.Data[1]--;
			Face.Data[2]--;
			Faces.push_back(Face);
		}
	}

	InputFile.close();

	return true;
}

// Write vertices/faces to a file
bool WriteColFile(const char *Filename) {

	// Open file
	ofstream File;
	File.open(Filename, ios::out | ios::binary);
	if(!File.is_open()) {
		printf("Error opening %s for writing\n", Filename);

		return false;
	}

	// Write header
	int VertCount = Vertices.size();
	int FaceCount = Faces.size();
	File.write((char *)&VertCount, sizeof(int));
	File.write((char *)&FaceCount, sizeof(int));

	// Write vertices
	for(int i = 0; i < VertCount; i++) {
		File.write((char *)&Vertices[i].Data[0], sizeof(float));
		File.write((char *)&Vertices[i].Data[1], sizeof(float));
		File.write((char *)&Vertices[i].Data[2], sizeof(float));
	}

	// Write faces
	for(int i = 0; i < FaceCount; i++) {
		File.write((char *)&Faces[i].Data[0], sizeof(int));
		File.write((char *)&Faces[i].Data[1], sizeof(int));
		File.write((char *)&Faces[i].Data[2], sizeof(int));
	}

	File.close();

	return true;
}
