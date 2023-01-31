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

void Mesh::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	GameFramework& gameFramework = GameFramework::Instance();
	if (!_file) {
		cout << "Mesh Load Failed" << "\n";
		return;
	}
	//  nVertex (UINT)
	_file.read((char*)&nVertex, sizeof(UINT));

	name = _obj->GetName();

	// boundingBox (float * 6)
	XMFLOAT3 oobbCenter, oobbExtents;
	_file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	_file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));
	_obj->SetBoundingBox(oobb);

	// positions (float * 3 * nVertex)
	vector<XMFLOAT3> positions(nVertex);
	_file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);


	// positions를 리소스로 만드는 과정
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// normals (float * 3 * nVertex)
	vector<float> normals(3 * nVertex);
	_file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals를 리소스로 만드는 과정
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pNormalUploadBuffer);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	// nTangent(UINT)
	UINT nTangent;
	_file.read((char*)&nTangent, sizeof(UINT));

	if (nTangent != nVertex) {
		cout << "에러 : nTangent와 nVertex 가 같지 않음\n";
	}
	if (nTangent > 0) {
		// tangents (float * 3 * nTangent)
		vector<float> tangents(3 * nTangent);
		_file.read((char*)tangents.data(), sizeof(float) * 3 * nTangent);
		// tangents를 리소스로 만드는 과정
		pTangentBuffer = CreateBufferResource(_pDevice, _pCommandList, tangents.data(), sizeof(float) * 3 * nTangent, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTangentUploadBuffer);
		tangentBufferView.BufferLocation = pTangentBuffer->GetGPUVirtualAddress();
		tangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
		tangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * nTangent;

		// tangents (float * 3 * nTangent)
		vector<float> biTangent(3 * nTangent);
		_file.read((char*)biTangent.data(), sizeof(float) * 3 * nTangent);
		// tangents를 리소스로 만드는 과정
		pBiTangentsBuffer = CreateBufferResource(_pDevice, _pCommandList, biTangent.data(), sizeof(float) * 3 * nTangent, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pBiTangentsUploadBuffer);
		biTangentsBufferView.BufferLocation = pBiTangentsBuffer->GetGPUVirtualAddress();
		biTangentsBufferView.StrideInBytes = sizeof(XMFLOAT3);
		biTangentsBufferView.SizeInBytes = sizeof(XMFLOAT3) * nTangent;
	}


	// 텍스처좌표값 읽기
	int nTexCoord = 0;
	_file.read((char*)&nTexCoord, sizeof(int));
	vector<float> texcoords;

	if (nTexCoord > 0) {
		// texcoord (float * 2 * nVertex)
		texcoords.resize(2 * nTexCoord);
		_file.read((char*)texcoords.data(), sizeof(float) * 2 * nTexCoord);
		// texture uv를 리소스로 만드는 과정
		pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, texcoords.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
		texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
		texCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
		texCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * nVertex;
	}

	// nSubMesh (UINT)
	UINT nSubMesh;
	_file.read((char*)&nSubMesh, sizeof(UINT));
	// 서브메쉬의 개수만큼 벡터를 미리 할당해 놓는다.

	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	pSubMeshIndexUploadBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});

	for (UINT i = 0; i < nSubMesh; ++i) {
		// nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
		_file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		_file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]);

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
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[5] = { positionBufferView , normalBufferView, tangentBufferView, biTangentsBufferView, texCoord0BufferView };
	_pCommandList->IASetVertexBuffers(0, 5, vertexBuffersViews);

	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], 1, 0, 0, 0);
}

void Mesh::RenderInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, Instancing_Data& _instanceData) {
	UINT numInstance = _instanceData.activeInstanceCount;

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	// 인자로 들어온 인스턴스 정보를 이용하여 갯수만큼 한꺼번에 그린다.
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[6] = { positionBufferView , normalBufferView, tangentBufferView, biTangentsBufferView,  texCoord0BufferView, _instanceData.bufferView };
	_pCommandList->IASetVertexBuffers(0, 6, vertexBuffersViews);
	  
	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], numInstance, 0, 0, 0);
}

//////////////// SkinnedMesh ///////////////////
void SkinnedMesh::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	_file.read((char*)&bonesPerVertex, sizeof(UINT));	// 뼈에 영향을 주는 정점의 개수
	_file.read((char*)&nBone, sizeof(UINT));			// 뼈의 개수

	// 오프셋 행렬들을 리소스로 만드는 과정 => 루트시그니쳐에 연결해야 한다.
	vector<XMFLOAT4X4> offsetMatrix(MAX_BONE);				// 오프셋 행렬들
	_file.read((char*)offsetMatrix.data(), sizeof(XMFLOAT4X4) * nBone);
	for (auto& matrix : offsetMatrix | views::take(nBone)) {
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMLoadFloat4x4(&matrix)));
	}
	
	UINT ncbElementBytes = ((sizeof(XMFLOAT4X4) * MAX_BONE + 255) & ~255); //256의 배수
	pOffsetMatrixBuffer = ::CreateBufferResource(_pDevice, _pCommandList, offsetMatrix.data(), ncbElementBytes, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pOffsetMatrixUploadBuffer);
	pOffsetMatrixBufferView.BufferLocation = pOffsetMatrixBuffer->GetGPUVirtualAddress();
	pOffsetMatrixBufferView.StrideInBytes = sizeof(XMFLOAT4X4);
	pOffsetMatrixBufferView.SizeInBytes = sizeof(XMFLOAT4X4) * MAX_BONE;

	// 스킨드 메쉬의 바운딩 박스
	XMFLOAT3 skinnedOOBBCenter, skinnedOOBBExtents;
	_file.read((char*)&skinnedOOBBCenter, sizeof(XMFLOAT3));
	_file.read((char*)&skinnedOOBBExtents, sizeof(XMFLOAT3));
	skinnedOOBB = BoundingOrientedBox(skinnedOOBBCenter, skinnedOOBBExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));

	// 메쉬 정보 읽기
	Mesh::LoadFromFile(_file, _pDevice, _pCommandList, _obj);

	// 정점의 뼈에대한 인덱스정보 리소스로 만들기
	// boneIndices (UINT * 4 * nVertex)
	vector<UINT> boneIndices(nVertex * bonesPerVertex);
	_file.read((char*)boneIndices.data(), sizeof(UINT) * bonesPerVertex * nVertex);

	pBoneIndexBuffer = CreateBufferResource(_pDevice, _pCommandList, boneIndices.data(), sizeof(UINT) * bonesPerVertex * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pBoneIndexUploadBuffer);
	boneIndexBufferView.BufferLocation = pBoneIndexBuffer->GetGPUVirtualAddress();
	boneIndexBufferView.StrideInBytes = sizeof(UINT) * bonesPerVertex;
	boneIndexBufferView.SizeInBytes = sizeof(UINT) * bonesPerVertex * nVertex;

	// 정점의 뼈에대한 가중치정보 리소스로 만들기
	// boneWeight (float * 4 * nVertex)
	vector<float> boneWeight(nVertex * bonesPerVertex);
	_file.read((char*)boneWeight.data(), sizeof(float) * bonesPerVertex * nVertex);

	pBoneWeightBuffer = CreateBufferResource(_pDevice, _pCommandList, boneWeight.data(), sizeof(float) * bonesPerVertex * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pBoneWeightUploadBuffer);
	boneWeightBufferView.BufferLocation = pBoneWeightBuffer->GetGPUVirtualAddress();
	boneWeightBufferView.StrideInBytes = sizeof(float) * bonesPerVertex;
	boneWeightBufferView.SizeInBytes = sizeof(float) * bonesPerVertex * nVertex;

}
void SkinnedMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex) {

	// 오프셋 행렬 루트시그니처에 연결
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pOffsetMatrixBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(6, gpuVirtualAddress);

	// 그리기
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[7] = { positionBufferView , normalBufferView, tangentBufferView, biTangentsBufferView,  texCoord0BufferView, boneIndexBufferView, boneWeightBufferView };
	_pCommandList->IASetVertexBuffers(0, 7, vertexBuffersViews);

	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], 1, 0, 0, 0);
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

FrustumMesh::FrustumMesh() {
}

FrustumMesh::~FrustumMesh() {
}

void FrustumMesh::Create(shared_ptr< BoundingFrustum> _pBoundingFrustum, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	BoundingFrustum& frustum = *_pBoundingFrustum;

	frustum.Orientation = Vector4::QuaternionIdentity();
	FRUSTUM_POSITION_FORMAT positions;
	positions.origin = frustum.Origin;
	XMFLOAT3* pCorner = &positions.nearPoint[0];
	frustum.GetCorners(pCorner);
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, &positions, sizeof(XMFLOAT3) * 9, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * 9;

	vector<UINT> indices{
		0,5,0,6,
		0,7,0,8,
		1,2,2,3,
		3,4,4,1,
		5,6,6,7,
		7,8,8,5,
	};

	pIndexBuffer = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pIndexUploadBuffer);
	indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(UINT) * 24;

}

void FrustumMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { positionBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);
	_pCommandList->IASetIndexBuffer(&indexBufferView);
	_pCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0);
}

void FrustumMesh::UpdateMesh(shared_ptr<BoundingFrustum> _pBoundingFrustum) {
	pPositionBuffer->Map(0, NULL, (void**)&pMappedFrustumMesh);
	pMappedFrustumMesh->origin = _pBoundingFrustum->Origin;
	XMFLOAT3* pCorner = &pMappedFrustumMesh->nearPoint[0];
	_pBoundingFrustum->GetCorners(pCorner);
	pPositionBuffer->Unmap(0, NULL);
}


