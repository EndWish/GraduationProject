#pragma once

class Mesh {
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	UINT mnVertex;	// ���ؽ�(������ ��ֺ���)�� ����
	ComPtr<ID3D12Resource> m_pPositionBuffer;	// 
	D3D12_VERTEX_BUFFER_VIEW m_positionBufferView;

	// ���̴� ������ [����] + ���׸����� ���� ���̴��� ������ ������Ʈ �ؾ��Ѵ�.

	BoundingOrientedBox m_OOBB;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& pd3dCommandList);
};

