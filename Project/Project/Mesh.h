#pragma once

#include "Shader.h"
#include "Material.h"

class Mesh {
protected:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
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

	vector<UINT> nSubMeshIndex;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	vector<shared_ptr<Material>> materials;
	BoundingOrientedBox oobb;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���

	// get, set�Լ�
	const string& GetName() const;
	const BoundingOrientedBox& GetOOBB() const;
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

// ������� ��� �ϴ� �޽�

// boundingBox �޽�
class HitBoxMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;
	
	ComPtr<ID3D12Resource> pIndexBuffers;	// subMesh���� �ε��� ����
	ComPtr<ID3D12Resource> pIndexUploadBuffers;
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
	map<string, shared_ptr<Mesh>> storage;
	HitBoxMesh hitBoxMesh;
public:
	HitBoxMesh& GetHitBoxMesh();
	shared_ptr<Mesh> GetMesh(const string& _name,const ComPtr<ID3D12Device>& _pDevice,const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};
