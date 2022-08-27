#include "stdafx.h"
#include "Mesh.h"
#include "GameFramework.h"

// ���� ���� �� �Լ�
shared_ptr<Shader> Mesh::shader;
shared_ptr<Shader> HitBoxMesh::shader;

void Mesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<BasicShader>(_pDevice, _pRootSignature);
}
shared_ptr<Shader> Mesh::GetShader() {
	return shader;
}

// ������, �Ҹ���
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

void Mesh::LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	ifstream file("Mesh/" + _fileName, ios::binary);	// ������ ����
	
	// ���ؽ��� ���� �б�
	file.read((char*)&nVertex, sizeof(UINT));
	// �� �̸� �ϱ�
	ReadStringBinary(name, file);

	// OOBB���� �б�
	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));
	if (_fileName == "Mesh_Floor01_Floor01") {
		cout << oobbCenter << ", " << oobbExtents << "\n";
	}

	// �����ǰ� �б�
	vector<float> positions(3 * nVertex);
	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions�� ���ҽ��� ����� ����
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;
	
	// ��ְ� �б�
	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals�� ���ҽ��� ����� ����
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pNormalUploadBuffer);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// ����޽� ���� �б�
	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	// ����޽��� ������ŭ ���͸� �̸� �Ҵ��� ���´�.
	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	pSubMeshIndexUploadBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});
	materials.assign(nSubMesh, {});

	for (UINT i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);
		// subMeshIndices�� ���ҽ��� ����� ����
		pSubMeshIndexBuffers[i] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pSubMeshIndexUploadBuffers[i]);
		subMeshIndexBufferViews[i].BufferLocation = pSubMeshIndexBuffers[i]->GetGPUVirtualAddress();
		subMeshIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		subMeshIndexBufferViews[i].SizeInBytes = sizeof(UINT) * nSubMeshIndex[i];
		
		// ���׸��� �������� �б�. (Ȯ���� ����)
		string materialName;
		ReadStringBinary(materialName, file);

		materials[i] = gameFramework.GetMaterialManager().GetMaterial(materialName, _pDevice, _pCommandList);
	}
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[2] = { positionBufferView , normalBufferView};
	_pCommandList->IASetVertexBuffers(0, 2, vertexBuffersViews);
	for (int i = 0; i < nSubMeshIndex.size(); ++i) {
		// �ش� ����Ž��� ��Ī�Ǵ� ���׸����� Set ���ش�.
		materials[i]->UpdateShaderVariable(_pCommandList);
		_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[i]);
		_pCommandList->DrawIndexedInstanced(nSubMeshIndex[i], 1, 0, 0, 0);
	}
}



void HitBoxMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<HitBoxShader>(_pDevice, _pRootSignature);
}

shared_ptr<Shader> HitBoxMesh::GetShader() {
	return shader;
}

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
	
	XMFLOAT3 c(0,0,0);
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

	pIndexBuffers = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pIndexUploadBuffers);
	indexBufferView.BufferLocation = pIndexBuffers->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(UINT) * 24;
	cout << "�������";
}





/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
MeshManager::MeshManager() {
	
}

MeshManager::~MeshManager() {

}

HitBoxMesh& MeshManager::GetHitBoxMesh() {
	return hitBoxMesh;
}

shared_ptr<Mesh> MeshManager::GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if(!storage.contains(_name)) {	// ó�� �ҷ��� �޽��� ���
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();
		newMesh->LoadFromFile(_name, _pDevice, _pCommandList);
		storage[_name] = newMesh;
	}
	return storage[_name];
}
