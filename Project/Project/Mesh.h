#pragma once

class Mesh {
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	UINT mnVertex;	// 버텍스(정점과 노멀벡터)의 개수
	ComPtr<ID3D12Resource> m_pPositionBuffer;	// 
	D3D12_VERTEX_BUFFER_VIEW m_positionBufferView;

	// 쉐이더 포인터 [수정] + 메테리얼을 통해 쉐이더의 정보를 업데이트 해야한다.

	BoundingOrientedBox m_OOBB;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Mesh();
	virtual ~Mesh();

public:		// 멤버 함수▼
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& pd3dCommandList);
};

