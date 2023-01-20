#pragma once

#include "Shader.h"
#include "Material.h"

struct Instancing_Data;
class GameObject;
class Particle;

class Mesh {

protected:
	UINT refCount;
	string name;


	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// 버텍스(정점과 노멀벡터)의 개수

	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// 노멀벡터의 정보
	ComPtr<ID3D12Resource> pNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// 텍스처 좌표의 정보
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	vector<UINT> nSubMeshIndex;	// subMesh들의 인덱스 개수
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh들의 인덱스 정보
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	BoundingOrientedBox oobb;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Mesh();
	virtual ~Mesh();

public:		// 멤버 함수▼

	// get, set함수
	const string& GetName() const;
	const BoundingOrientedBox& GetOOBB() const;
	void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
	void AddRef();
	UINT GetRef();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex);
	virtual void RenderInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, Instancing_Data& _instanceData);
};

// boundingBox 메쉬
class HitBoxMesh {
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// 인덱스 정보
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	HitBoxMesh();
	~HitBoxMesh();
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

struct FRUSTUM_POSITION_FORMAT {
	XMFLOAT3 origin;
	XMFLOAT3 nearPoint[4];
	XMFLOAT3 farPoint[4];
};

class FrustumMesh {
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보. Upload상태
	ComPtr<ID3D12Resource> pPositionUploadBuffer;	// 버텍스의 위치 정보. Upload상태
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;
	
	shared_ptr<FRUSTUM_POSITION_FORMAT> pMappedFrustumMesh;

	ComPtr<ID3D12Resource> pIndexBuffer;	// 인덱스 정보
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	FrustumMesh();
	~FrustumMesh();
	void Create(shared_ptr< BoundingFrustum> _pBoundingFrustum, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateMesh(shared_ptr< BoundingFrustum> _pBoundingFrustum);
};