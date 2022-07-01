#pragma once

#include "Material.h"

class Mesh {
protected:	// 멤버 변수▼
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	
	UINT m_nVertices;

	ComPtr<ID3D12Resource> m_pPositionBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_positionBufferView;

	vector<shared_ptr<Material>> m_pMaterials;

	// 쉐이더 포인터 [수정] + 메테리얼을 통해 쉐이더의 정보를 업데이트 해야한다.

	BoundingOrientedBox m_OOBB;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Mesh();
	virtual ~Mesh();

public:		// 멤버 함수▼
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};

class IndexingMesh : public Mesh {
protected:	// 멤버 변수▼
	// 여러 SubSet으로 나뉘는 이유는 하나의 메쉬에 여러 재질이 섞여 있을 수 있기 때문이다. (예를 들어 자동차메쉬에 보닛은 광택재질, 윈드실드(앞쪽창문)는 유리, 바퀴는 타이어 등등 재질이 각각다르다.) 
	vector<int> m_subSetIndices;	// subSet들의 인덱스 개수
	vector<ComPtr<ID3D12Resource>> m_pSubSetIndexBuffers;	// subSet들의 인덱스 버퍼
	vector<D3D12_INDEX_BUFFER_VIEW> m_subSetIndexBufferViews;	// subSet들의 인덱스 버퍼뷰

public:		// 생성관련 멤버 함수▼
	IndexingMesh();
	virtual ~IndexingMesh();

public:		// 멤버 함수▼
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};

class NormalIndexingMesh : public IndexingMesh {
protected:	// 멤버 변수▼
	ComPtr<ID3D12Resource> m_pNormalBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_normalBufferView;

public:		// 생성관련 멤버 함수▼
	NormalIndexingMesh();
	virtual ~NormalIndexingMesh();

public:		// 멤버 함수▼
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};
