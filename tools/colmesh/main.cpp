/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2019  Alan Witkowski
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
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <map>

// Face struct
struct _Face {
	_Face() { }
	_Face(int Index1, int Index2, int Index3) : Data({Index1, Index2, Index3}) { }
	int &operator[](std::size_t Index) { return Data[Index]; }

	std::array<int, 3> Data;
};

// Vertex struct
struct _Vertex {
	_Vertex() { }
	_Vertex(float X, float Y, float Z) : Data({X, Y, Z}) { }
	bool operator<(const _Vertex &Value) const { return std::tie(Data[0], Data[1], Data[2]) < std::tie(Value.Data[0], Value.Data[1], Value.Data[2]); }
	const float &operator[](std::size_t Index) const { return Data[Index]; }

	std::array<float, 3> Data;
};

// Globals
static std::vector<_Vertex> Vertices;
static std::vector<_Face> Faces;

// Functions
static bool ReadObjFile(const char *Filename);
static bool WriteColFile(const char *Filename);

int main(int ArgumentCount, char **Arguments) {

	// Parse arguments
	if(ArgumentCount != 2) {
		std::cout << "Needs 1 argument: .obj file" << std::endl;
		return EXIT_FAILURE;
	}

	// Parse file
	std::string File = Arguments[1];
	size_t Extension = File.rfind(".obj");
	if(Extension == std::string::npos) {
		std::cout << "Bad argument: " << Arguments[1] << std::endl;
		return EXIT_FAILURE;
	}

	// Get filenames
	std::string BaseName = File.substr(0, Extension);
	std::string ObjFilename = BaseName + std::string(".obj");
	std::string ColFilename = BaseName + std::string(".col");

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
	std::ifstream InputFile(Filename);
	if(!InputFile.is_open()) {
		std::cout << "Error opening '" << Filename << "' for reading" << std::endl;

		return false;
	}

	// Read file
	char Buffer[256];
	bool HasTexture = false;
	int NewVertexIndex = 0;
	std::map<_Vertex, int> VertexMap;
	std::vector<_Vertex> TempVertices;
	while(!InputFile.eof()) {
		InputFile.getline(Buffer, 255);

		// Read vertices
		if(Buffer[0] == 'v') {
			if(Buffer[1] == ' ') {
				_Vertex Vertex;

				sscanf(Buffer, "v %f %f %f", &Vertex.Data[0], &Vertex.Data[1], &Vertex.Data[2]);
				TempVertices.push_back(Vertex);
			}
			else if(Buffer[1] == 't') {
				HasTexture = true;
			}
		}
		// Read faces
		else if(Buffer[0] == 'f' && Buffer[1] == ' ') {
			_Face Face;
			int Dummy;

			if(HasTexture)
				sscanf(Buffer, "f %d/%d %d/%d %d/%d", &Face.Data[0], &Dummy, &Face.Data[1], &Dummy, &Face.Data[2], &Dummy);
			else
				sscanf(Buffer, "f %d %d %d", &Face.Data[0], &Face.Data[1], &Face.Data[2]);

			Face.Data[0]--;
			Face.Data[1]--;
			Face.Data[2]--;

			// Eliminate duplicate vertices
			for(int i = 0; i < 3; i++) {
				const _Vertex &Vertex = TempVertices[Face.Data[i]];
				auto Iterator = VertexMap.find(Vertex);
				if(Iterator == VertexMap.end()) {
					Vertices.push_back(Vertex);
					VertexMap[Vertex] = NewVertexIndex++;
				}

				// Get new vertex index
				Face.Data[i] = VertexMap[Vertex];
			}

			Faces.push_back(Face);
		}
	}

	// Close file
	InputFile.close();

	return true;
}

// Write vertices/faces to a binary file
bool WriteColFile(const char *Filename) {

	// Open file
	std::ofstream File;
	File.open(Filename, std::ios::out | std::ios::binary);
	if(!File.is_open()) {
		std::cout << "Error opening '" << Filename << "' for writing" << std::endl;

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

	// Close file
	File.close();

	return true;
}
