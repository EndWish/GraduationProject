#pragma once

class Mesh {
protected:
	string name;

	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	UINT nVertex;	// ���ؽ�(������ ��ֺ���)�� ����
	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// ��ֺ����� ����
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	vector<UINT> nSubMeshIndex;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh���� �ε��� ����
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	BoundingOrientedBox oobb;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���
	shared_ptr<Mesh> LoadFromFile(string& _fileName, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pd3dCommandList);
};

// ������� ��� �ϴ� �޽�
// boundingBox �޽�

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {

protected:
	map<string, shared_ptr<Mesh>> storage;

public:
	shared_ptr<Mesh> GetMesh(string& _name, ComPtr<ID3D12Device>& _pDevice, ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};