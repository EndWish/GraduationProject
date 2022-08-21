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
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

// ������� ��� �ϴ� �޽�
// boundingBox �޽�

/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {

protected:
	map<string, shared_ptr<Mesh>> storage;

public:
	shared_ptr<Mesh> GetMesh(const string& _name,const ComPtr<ID3D12Device>& _pDevice,const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};
