#include "stdafx.h"
#include "Mesh.h"



Mesh::Mesh() {

}
Mesh::~Mesh() {

}

shared_ptr<Mesh> Mesh::LoadFromFile(string& _fileName, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	ifstream file("Model/" + _fileName, ios::binary);	// 파일을 연다
	
	// 버텍스의 개수 읽기
	file.read((char*)&nVertex, sizeof(UINT));
	
	// 모델 이름 일기
	ReadStringBinary(name, file);

	// OOBB정보 읽기
	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));

	// 포지션값 읽기
	vector<float> positions(3 * nVertex);
	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions를 리소스로 만드는 과정
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// 노멀값 읽기
	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals를 리소스로 만드는 과정
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// 서브메쉬 정보 읽기
	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	// 서브메쉬의 개수만큼 벡터를 미리 할당해 놓는다.
	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});
	for (UINT i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);
		// subMeshIndices를 리소스로 만드는 과정
		pSubMeshIndexBuffers[i] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		subMeshIndexBufferViews[i].BufferLocation = pSubMeshIndexBuffers[i]->GetGPUVirtualAddress();
		subMeshIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		subMeshIndexBufferViews[i].SizeInBytes = sizeof(UINT) * nSubMeshIndex[i];
	}
	
}
void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pd3dCommandList) {
	cout << format("Mesh({}) : Mesh함수 수행\n", name);
}

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager



shared_ptr<Mesh> MeshManager::GetMesh(string& _name, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if(storage.contains(_name)) {	// 처음 불러온 메쉬일 경우
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();
		newMesh->LoadFromFile(_name, _pDevice, _pCommandList);
		storage[_name] = newMesh;
	}
	return storage[_name];
}