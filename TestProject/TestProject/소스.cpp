#include <iostream>
#include <fstream>
#include <random>
#include <ranges>
#include <format>
#include <memory.h>
#include <memory>

#include <vector>
#include <chrono>

#define UINT unsigned int

using namespace std;



void ReadModel(string _fileName, string _outName) {
	ifstream in(_fileName);
	ofstream out(_outName, ios::binary);
	UINT nVertex;
	in >> nVertex;
	out.write((char*)&nVertex, sizeof(UINT));

	UINT nameSize;
	in >> nameSize;
	out.write((char*)&nameSize, sizeof(UINT));

	string name;
	in >> name;
	out.write((char*)name.c_str(), sizeof(char) * nameSize);

	float boundingBox[6];
	for (int i = 0; i < 6; ++i) {
		in >> boundingBox[i];
		out.write((char*)&boundingBox[i], sizeof(float));
	}

	vector<float> positions(3 * nVertex);
	for (int i = 0; i < 3 * nVertex; ++i) {
		in >> positions[i];
		out.write((char*)&positions[i], sizeof(float));
	}

	vector<float> normals(3 * nVertex);
	for (int i = 0; i < 3 * nVertex; ++i) {
		in >> normals[i];
		out.write((char*)&normals[i], sizeof(float));
	}

	UINT nSubMesh;
	in >> nSubMesh;
	out.write((char*)&nSubMesh, sizeof(UINT));


	for (int i = 0; i < nSubMesh; ++i) {
		UINT nSubMeshIndex;
		in >> nSubMeshIndex;
		out.write((char*)&nSubMeshIndex, sizeof(UINT));

		vector<UINT> subMeshIndex(nSubMeshIndex);
		for (int j = 0; j < nSubMeshIndex; ++j) {
			in >> subMeshIndex[j];
			out.write((char*)&subMeshIndex[j], sizeof(UINT));
		}
		UINT matnameSize;
		in >> matnameSize;
		out.write((char*)&matnameSize, sizeof(UINT));

		string matname;
		in >> matname;
		out.write((char*)matname.c_str(), sizeof(char) * matnameSize);

	}

}

void ReadMaterial(string _fileName, string _outName) {
	ifstream in(_fileName);
	ofstream out(_outName, ios::binary);
	float materialInfo;
	for (int i = 0; i < 16; ++i) {
		in >> materialInfo;
		out.write((char*)&materialInfo, sizeof(float));
	}
}


int main()
{

	ReadModel("rect.txt", "rect");
	ReadMaterial("red.txt", "red");
	ReadMaterial("blue.txt", "blue");
	

}
