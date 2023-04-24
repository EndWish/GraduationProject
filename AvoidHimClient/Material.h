#pragma once
#include "Texture.h"

struct VS_MaterialMappedFormat {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 emissive;
	UINT nTypes;
};

class Material {
private:
	shared_ptr<Texture> pTexture;	// 알베도맵에 대한 텍스처
	shared_ptr<Texture> pBumpTexture; // 노말맵에 대한 텍스처 
	shared_ptr<Texture> pEmissiveTexture; // 발광맵에 대한 텍스처 

	shared_ptr<VS_MaterialMappedFormat> pMappedMaterial;
	ComPtr<ID3D12Resource> pMaterialBuffer;
	UINT nType = 0;			// 쉐이더 내에서 mask값을 위한 변수

public:
	Material();
	~Material();
	void DefaultMaterial(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadMaterial(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void SetTexture(shared_ptr<Texture> _pTexture);
	void SetBumpTexture(shared_ptr<Texture> _pBumpTexture);
	void SetEmissiveTexture(shared_ptr<Texture> _pEmissiveTexture);
	void SetDiffuse(const XMFLOAT4& _diffuse);
	void SetSpecular(const XMFLOAT4& _specular);
	void SetEmissive(const XMFLOAT4& _emissive);
};

