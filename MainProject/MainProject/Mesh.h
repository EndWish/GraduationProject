#pragma once

class Mesh {
protected:	// ��� ������
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	
	UINT m_nVertices;

	ComPtr<ID3D12Resource> m_positionBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_positionBufferView;

	BoundingOrientedBox m_OOBB;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};

class IndexingMesh : public Mesh {
protected:	// ��� ������
	// ���� SubSet���� ������ ������ �ϳ��� �޽��� ���� ������ ���� ���� �� �ֱ� �����̴�. (���� ��� �ڵ����޽��� ������ ��������, ����ǵ�(����â��)�� ����, ������ Ÿ�̾� ��� ������ �����ٸ���.) 
	vector<int> m_subSetIndices;	// subSet���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> m_pSubSetIndexBuffers;	// subSet���� �ε��� ����
	vector<D3D12_INDEX_BUFFER_VIEW> m_subSetIndexBufferViews;	// subSet���� �ε��� ���ۺ�

public:		// �������� ��� �Լ���
	IndexingMesh();
	virtual ~IndexingMesh();

public:		// ��� �Լ���
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};

class NormalIndexingMesh : public IndexingMesh {
protected:	// ��� ������
	ComPtr<ID3D12Resource> m_normalBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_normalBufferView;

public:		// �������� ��� �Լ���
	NormalIndexingMesh();
	virtual ~NormalIndexingMesh();

public:		// ��� �Լ���
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};
