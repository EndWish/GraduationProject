#include "stdafx.h"
#include "Material.h"
#include "GameFramework.h"

Material::Material() {

}

Material::~Material() {

}

void Material::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pMaterialBuffer->GetGPUVirtualAddress();
	gameFramework.GetCommandList()->SetGraphicsRootConstantBufferView(3, gpuVirtualAddress);
}

void Material::LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 마테리얼 내에 리소스 생성후 map
	shared_ptr<VS_MaterialMappedFormat> pMappedMaterial;

	GameFramework& gameFramework = GameFramework::Instance();
	UINT cbElementSize = (sizeof(VS_MaterialMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;
	pMaterialBuffer = CreateBufferResource(gameFramework.GetDevice(), gameFramework.GetCommandList(), NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pMaterialBuffer->Map(0, NULL, (void**)&pMappedMaterial);
	
	ifstream file("Material/" + _fileName, ios::binary);
	XMFLOAT4 ambient, diffuse, specular, emissive;

	// format에 맞는값을 파일에서 읽어 리소스에 복사
	file.read((char*)&ambient, sizeof(XMFLOAT4));
	file.read((char*)&diffuse, sizeof(XMFLOAT4));
	file.read((char*)&specular, sizeof(XMFLOAT4));
	file.read((char*)&emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->ambient, &ambient, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->diffuse, &diffuse, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->specular, &specular, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->emissive, &emissive, sizeof(XMFLOAT4));
}




/////////////////////////// MaterialManager ////////////////////////


shared_ptr<Material> MaterialManager::GetMaterial(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (!storage.contains(_name)) {	// 처음 불러온 마테리얼일 경우
		shared_ptr<Material> newMaterial = make_shared<Material>();
		newMaterial->LoadFromFile(_name, _pDevice, _pCommandList);
		storage[_name] = newMaterial;
	}
	return storage[_name];
}

