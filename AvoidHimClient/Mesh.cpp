#include "stdafx.h"
#include "Mesh.h"
#include "GameFramework.h"


// 생성자, 소멸자
Mesh::Mesh() {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}
Mesh::~Mesh() {

}

const string& Mesh::GetName() const {
	return name;
}

const BoundingOrientedBox& Mesh::GetOOBB() const {
	return oobb;
}

void Mesh::LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	GameFramework& gameFramework = GameFramework::Instance();
	ifstream file("Mesh/" + _fileName, ios::binary);	// 파일을 연다
	if (!file) {
		cout << "Mesh Load Failed : " << _fileName << "\n";
		return;
	}
	// 버텍스의 개수 읽기
	file.read((char*)&nVertex, sizeof(UINT));
	// 모델 이름 일기
	ReadStringBinary(name, file);

	// OOBB정보 읽기
	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));
	_obj->SetBoundingBox(oobb);
	// 포지션값 읽기
	vector<XMFLOAT3> positions(nVertex);

	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions를 리소스로 만드는 과정

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// 노멀값 읽기
	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals를 리소스로 만드는 과정
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pNormalUploadBuffer);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;


	// 텍스처좌표값 읽기
	int nTexCoord = 0;
	file.read((char*)&nTexCoord, sizeof(int));

	vector<float> texcoords(2 * nTexCoord);
	file.read((char*)texcoords.data(), sizeof(float) * 2 * nTexCoord);

	// texture uv를 리소스로 만드는 과정
	pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, texcoords.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
	texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
	texCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	texCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * nVertex;

	// 서브메쉬 정보 읽기
	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	// 서브메쉬의 개수만큼 벡터를 미리 할당해 놓는다.

	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	pSubMeshIndexUploadBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});

	for (UINT i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);

		// subMeshIndices를 리소스로 만드는 과정
		pSubMeshIndexBuffers[i] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pSubMeshIndexUploadBuffers[i]);
		subMeshIndexBufferViews[i].BufferLocation = pSubMeshIndexBuffers[i]->GetGPUVirtualAddress();
		subMeshIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		subMeshIndexBufferViews[i].SizeInBytes = sizeof(UINT) * nSubMeshIndex[i];
	}
}

void Mesh::ReleaseUploadBuffers() {
	if (pPositionUploadBuffer) pPositionUploadBuffer->Release();
	if (pNormalUploadBuffer) pNormalUploadBuffer->Release();
	if (pTexCoord0UploadBuffer) pTexCoord0UploadBuffer->Release();
	for (auto& pSubMeshIndexUploadBuffer : pSubMeshIndexUploadBuffers) {
		pSubMeshIndexUploadBuffer->Release();
	}
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex) {

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[3] = { positionBufferView , normalBufferView, texCoord0BufferView };
	_pCommandList->IASetVertexBuffers(0, 3, vertexBuffersViews);


	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], 1, 0, 0, 0);
}

void Mesh::RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, const D3D12_VERTEX_BUFFER_VIEW& _instanceBufferView, int _numInstance) {

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);

	// 인자로 들어온 인스턴스 정보를 이용하여 갯수만큼 한꺼번에 그린다.
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[4] = { positionBufferView , normalBufferView, texCoord0BufferView, _instanceBufferView };
	_pCommandList->IASetVertexBuffers(0, 4, vertexBuffersViews);

	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], _numInstance, 0, 0, 0);
}

//////////////// HitBoxMesh ///////////////////


HitBoxMesh::HitBoxMesh() {

}

HitBoxMesh::~HitBoxMesh() {

}

void HitBoxMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { positionBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);
	_pCommandList->IASetIndexBuffer(&indexBufferView);
	_pCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0);
}

void HitBoxMesh::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	XMFLOAT3 c(0, 0, 0);
	XMFLOAT3 e(0.5, 0.5, 0.5);
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	vector<XMFLOAT3> positions{
		XMFLOAT3(c.x - e.x, c.y - e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y - e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y - e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y - e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y + e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y + e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y + e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y + e.y, c.z - e.z),
	};

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(XMFLOAT3) * 8, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * 8;

	vector<UINT> indices{
		0,1,1,2,
		2,3,3,0,
		4,5,5,6,
		6,7,7,4,
		0,4,1,5,
		2,6,3,7
	};

	pIndexBuffer = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pIndexUploadBuffer);
	indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(UINT) * 24;

}

//////////////// TerrainMesh ////////////////////


/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
MeshManager::MeshManager() {

}

MeshManager::~MeshManager() {

}

HitBoxMesh& MeshManager::GetHitBoxMesh() {
	return hitBoxMesh;
}

shared_ptr<Mesh> MeshManager::GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	if (!storage.contains(_name)) {	// 처음 불러온 메쉬일 경우
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();

		newMesh->LoadFromFile(_name, _pDevice, _pCommandList, _obj);

		storage[_name] = newMesh;
	}
	return storage[_name];
}

void MeshManager::ReleaseUploadBuffers() {
	for (auto& pMesh : storage) {
		pMesh.second->ReleaseUploadBuffers();
	}
}

