#pragma once

#include "Shader.h"
#include "Material.h"

class GameObject;
class Particle;

class Mesh {

protected:
	string name;


	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// ���ؽ�(������ ��ֺ���)�� ����

	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// ��ֺ����� ����
	ComPtr<ID3D12Resource> pNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// �ؽ�ó ��ǥ�� ����
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	vector<UINT> nSubMeshIndex;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	BoundingOrientedBox oobb;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���

	// get, set�Լ�
	const string& GetName() const;
	const BoundingOrientedBox& GetOOBB() const;
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex);
	virtual void RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, const D3D12_VERTEX_BUFFER_VIEW& _instanceBufferView, int _numInstance);
};

// ������� ��� �ϴ� �޽�

// boundingBox �޽�
class HitBoxMesh {
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// �ε��� ����
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	HitBoxMesh();
	~HitBoxMesh();
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {
public:
	MeshManager();
	~MeshManager();

protected:
	unordered_map<string, shared_ptr<Mesh>> storage;
	HitBoxMesh hitBoxMesh;
public:
	HitBoxMesh& GetHitBoxMesh();
	shared_ptr<Mesh> GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
};
