#pragma once

#include "Shader.h"
#include "Material.h"

class Mesh {
protected:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader();
	static shared_ptr<Shader> GetShader();
protected:
	string name;


	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	UINT nVertex;	// 버텍스(정점과 노멀벡터)의 개수
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// 노멀벡터의 정보
	ComPtr<ID3D12Resource> pNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	vector<UINT> nSubMeshIndex;	// subMesh들의 인덱스 개수
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh들의 인덱스 정보
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	vector<shared_ptr<Material>> materials;
	BoundingOrientedBox oobb;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Mesh();
	virtual ~Mesh();

public:		// 멤버 함수▼
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

// 조명까지 계산 하는 메쉬
// boundingBox 메쉬

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {

protected:
	map<string, shared_ptr<Mesh>> storage;

public:
	shared_ptr<Mesh> GetMesh(const string& _name,const ComPtr<ID3D12Device>& _pDevice,const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};
