#include "stdafx.h"
#include "Material.h"
#include "GameFramework.h"

Material::Material() {
	nType = 0;
}

Material::~Material() {
	pMaterialBuffer->Unmap(0, NULL);
}

void Material::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// 기본값들을 쓰지 않는다면 보내지 않는다.
	if (pMaterialBuffer) {

		D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pMaterialBuffer->GetGPUVirtualAddress();
		_pCommandList->SetGraphicsRootConstantBufferView(3, gpuVirtualAddress);
	}
	vector<shared_ptr<Texture>> pTextures{ pTexture, pBumpTexture, pEmissiveTexture };
	for (auto& pTex : pTextures) {
			
		if (pTex) {
			// 여기서 텍스처 정보를 쉐이더에 올림
			pTex->UpdateShaderVariable(_pCommandList);
		}
	}
}

void Material::LoadMaterial(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 마테리얼 내에 리소스 생성후 map

	GameFramework& gameFramework = GameFramework::Instance();

	// albedoNameSize(UINT) / albedoName(string) -> 알베도 텍스처 이름
	// bumpNameSize(UINT) / bumpName(string) -> 노말맵 텍스처 이름

	vector<int> rootParameterIndice{4, 5, 12};
	vector<shared_ptr<Texture>> pTextures(3);
	string textureName;

	for (int i = 0; auto & pTex : pTextures) {

		ReadStringBinary(textureName, _file);

		if (textureName != "null") {
			pTex = gameFramework.GetTextureManager().GetTexture(textureName, _pDevice, _pCommandList, rootParameterIndice[i]);

			if (pTex) {
				nType |= 1 << i;
			}
			else {
				cout << textureName << "로드 실패..\n";
			}
		}

		
		++i;
	}

	pTexture = pTextures[0];
	pBumpTexture = pTextures[1];
	pEmissiveTexture = pTextures[2];

	
	UINT cbElementSize = (sizeof(VS_MaterialMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;
	pMaterialBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pMaterialBuffer->Map(0, NULL, (void**)&pMappedMaterial);

	XMFLOAT4 ambient, diffuse, specular, emissive;
	XMFLOAT4 mp = XMFLOAT4(0, 0, 0, 1);
	// format에 맞는값을 파일에서 읽어 리소스에 복사
	_file.read((char*)&ambient, sizeof(XMFLOAT4));
	_file.read((char*)&diffuse, sizeof(XMFLOAT4));
	_file.read((char*)&specular, sizeof(XMFLOAT4));
	_file.read((char*)&emissive, sizeof(XMFLOAT4));
	   
	diffuse = XMFLOAT4(diffuse.x / 2, diffuse.y / 2, diffuse.z / 2, diffuse.w);

	memcpy(&pMappedMaterial->ambient, &ambient, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->diffuse, &diffuse, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->specular, &specular, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->emissive, &emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->nTypes, &nType, sizeof(UINT));
}

void Material::SetTexture(shared_ptr<Texture> _pTexture) {
	if (_pTexture) nType |= 1;
	else nType &= ~1;
	pTexture = _pTexture;
}

void Material::SetBumpTexture(shared_ptr<Texture> _pBumpTexture) {
	if (_pBumpTexture) nType |= 2;
	else nType &= ~2;
	pBumpTexture = _pBumpTexture;
}

void Material::SetEmissiveTexture(shared_ptr<Texture> _pEmissiveTexture) {
	if (_pEmissiveTexture) 
		nType |= 4;
	else 
		nType &= ~4;
	pEmissiveTexture = _pEmissiveTexture;
}

void Material::SetDiffuse(const XMFLOAT4& _diffuse) {
	pMappedMaterial->diffuse = _diffuse;
}

void Material::SetSpecular(const XMFLOAT4& _specular) {
	pMappedMaterial->specular = _specular;
}

void Material::SetEmissive(const XMFLOAT4& _emissive) {
	pMappedMaterial->emissive = _emissive;
}




void Material::DefaultMaterial(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	UINT cbElementSize = (sizeof(VS_MaterialMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;

	pMaterialBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pMaterialBuffer->Map(0, NULL, (void**)&pMappedMaterial);

	XMFLOAT4 ambient = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 diffuse = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 specular = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 emissive = XMFLOAT4(1, 1, 1, 1);

	// format에 맞는값을 파일에서 읽어 리소스에 복사

	memcpy(&pMappedMaterial->ambient, &ambient, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->diffuse, &diffuse, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->specular, &specular, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->emissive, &emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->nTypes, &nType, sizeof(UINT));
}