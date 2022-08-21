#pragma once

struct VS_MaterialMappedFormat {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 emissive;
};

class Material {
private:
	ComPtr<ID3D12Resource> pMaterialBuffer;
	
public:
	Material();
	~Material();

	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class MaterialManager {
	map<string, shared_ptr<Material>> storage;

public:

	shared_ptr<Material> GetMaterial(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};