#include "stdafx.h"
#include "Mesh.h"



Mesh::Mesh() {

}
Mesh::~Mesh() {

}

shared_ptr<Mesh> Mesh::LoadFromFile(string& _fileName) {
	ifstream file("Model/" + _fileName, ios::binary);	// ������ ����
	
	file.read((char*)&nVertex, sizeof(UINT));
	
	ReadStringBinary(name, file);

	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));

	vector<float> positions(3 * nVertex);
	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions�� ���ҽ��� ����� ���� [���� ���]

	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals�� ���ҽ��� ����� ���� [���� ���]

	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	nSubMeshIndex.assign(nSubMesh, 0);
	//pSubMeshIndexBuffers.clear();
	//subMeshIndexBufferViews.clear();
	for (int i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);
		// subMeshIndices�� ���ҽ��� ����� ���� [���� ���]
	}
	
}
void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pd3dCommandList) {
	cout << format("Mesh({}) : Mesh�Լ� ����\n", name);
}

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager



shared_ptr<Mesh> MeshManager::GetMesh(string& _name) {
	if(storage.contains(_name)) {	// ó�� �ҷ��� �޽��� ���
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();
		newMesh->LoadFromFile(_name);
		storage[_name] = newMesh;
	}
	return storage[_name];
}