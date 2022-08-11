#include "stdafx.h"
#include "Mesh.h"



Mesh::Mesh() {

}
Mesh::~Mesh() {

}

shared_ptr<Mesh> Mesh::LoadFromFile(string& _fileName, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	ifstream file("Model/" + _fileName, ios::binary);	// ������ ����
	
	// ���ؽ��� ���� �б�
	file.read((char*)&nVertex, sizeof(UINT));
	
	// �� �̸� �ϱ�
	ReadStringBinary(name, file);

	// OOBB���� �б�
	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));

	// �����ǰ� �б�
	vector<float> positions(3 * nVertex);
	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions�� ���ҽ��� ����� ����
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// ��ְ� �б�
	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals�� ���ҽ��� ����� ����
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// ����޽� ���� �б�
	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	// ����޽��� ������ŭ ���͸� �̸� �Ҵ��� ���´�.
	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});
	for (UINT i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);
		// subMeshIndices�� ���ҽ��� ����� ����
		pSubMeshIndexBuffers[i] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		subMeshIndexBufferViews[i].BufferLocation = pSubMeshIndexBuffers[i]->GetGPUVirtualAddress();
		subMeshIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		subMeshIndexBufferViews[i].SizeInBytes = sizeof(UINT) * nSubMeshIndex[i];
	}
	
}
void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pd3dCommandList) {
	cout << format("Mesh({}) : Mesh�Լ� ����\n", name);
}

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager



shared_ptr<Mesh> MeshManager::GetMesh(string& _name, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if(storage.contains(_name)) {	// ó�� �ҷ��� �޽��� ���
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();
		newMesh->LoadFromFile(_name, _pDevice, _pCommandList);
		storage[_name] = newMesh;
	}
	return storage[_name];
}