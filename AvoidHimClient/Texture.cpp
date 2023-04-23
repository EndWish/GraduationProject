#include "stdafx.h"
#include "Texture.h"
#include "GameObject.h"
#include "GameFramework.h"

Texture::Texture(int _nTexture, UINT _textureType, int _nSampler, int _nRootParameter) {

	// nTextures : 텍스처의 개수
	// nTextureType : 텍스처의 타입
	// nSamplers : 샘플러의 개수. 정적 샘플러를 사용시 여기는 사용하지 않는다
	// nRootParameters : 루트 파라미터의 개수
	// 알베도, 노말맵을 사용하게 된다면 Texture(2, RESOURCE_TEXTURE2D, 0, 1);이 될것이다.
	textureType = _textureType;

	nTexture = _nTexture;
	if (nTexture > 0)
	{
		pTextureBuffers.assign(_nTexture, {});
		pTextureUploadBuffers.assign(_nTexture, {});
		name.assign(_nTexture, {});

		srvGpuDescriptorHandles.resize(_nTexture);
		uavGpuDescriptorHandles.resize(_nTexture);
		srvComputeGpuDescriptorHandles.resize(_nTexture);
		uavComputeGpuDescriptorHandles.resize(_nTexture);

		for (int i = 0; i < _nTexture; i++) srvGpuDescriptorHandles[i].ptr = NULL;

		resourceTypes.resize(_nTexture);
		bufferFormats.resize(_nTexture);
		bufferElements.resize(_nTexture);
		bufferStrides.resize(_nTexture);
	}

	nRootParameter = _nRootParameter;

	if (nRootParameter > 0) rootParameterIndices.resize(nRootParameter);

	for (int i = 0; i < nRootParameter; i++) rootParameterIndices[i] = -1;

	nSampler = _nSampler;
	if (nSampler > 0) samplerGpuDescriptorHandles.resize(_nSampler);
}

Texture::~Texture() {

}

void Texture::SetRootParameterIndex(int _index, UINT _nRootParameterIndex)
{
	rootParameterIndices[_index] = _nRootParameterIndex;
}


void Texture::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList) {
	if (nRootParameter == nTexture)
	{
		for (int i = 0; i < nRootParameter; i++)
		{
			if (srvGpuDescriptorHandles[i].ptr && (rootParameterIndices[i] != -1)) {
				// 해당 번째의 루트 파라미터 인덱스, 서술자 힙의 인덱스 
				_pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndices[i], srvGpuDescriptorHandles[i]);
			}
		}	
	}
	else
	{
		if (srvGpuDescriptorHandles[0].ptr) {
			_pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndices[0], srvGpuDescriptorHandles[0]);
		}
	}
}

void Texture::UpdateComputeShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList, int _srvIndex, int _uavIndex) {

	if (nRootParameter == nTexture)
	{
		for (int i = 0; i < nRootParameter; i++)
		{
			if (srvComputeGpuDescriptorHandles[i].ptr && _srvIndex != -1) {
				_pCommandList->SetComputeRootDescriptorTable(_srvIndex, srvComputeGpuDescriptorHandles[i]);
			}
			if (uavComputeGpuDescriptorHandles[i].ptr && _uavIndex != -1) {
				_pCommandList->SetComputeRootDescriptorTable(_uavIndex, uavComputeGpuDescriptorHandles[i]);
			}
		}
	}
	else
	{
		if (srvComputeGpuDescriptorHandles[0].ptr && _srvIndex != -1) {
			_pCommandList->SetComputeRootDescriptorTable(_srvIndex, srvComputeGpuDescriptorHandles[0]);
		}
		if (uavComputeGpuDescriptorHandles[0].ptr && _uavIndex != -1) {

			_pCommandList->SetComputeRootDescriptorTable(_uavIndex, uavComputeGpuDescriptorHandles[0]);
		}
	}
}

void Texture::UpdateComputeShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList, int _srvIndex, int _uavIndex, int _index) {

	if (srvComputeGpuDescriptorHandles[_index].ptr && _srvIndex != -1) {
		cout << _index << "는 " << GetResource(_index).Get() << "입니다.\n";
		_pCommandList->SetComputeRootDescriptorTable(_srvIndex, srvComputeGpuDescriptorHandles[_index]);
	}
	if (uavComputeGpuDescriptorHandles[_index].ptr && _uavIndex != -1) {
		_pCommandList->SetComputeRootDescriptorTable(_uavIndex, uavComputeGpuDescriptorHandles[_index]);
	}

}

bool Texture::LoadFromFile(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _resourceType, int _index, int _startRootSignatureIndex) {

	// 텍스처 로드
	// 알베도 맵만 사용.
	name[_index] = _name;

	wstring tmpName = wstring(name[_index].begin(), name[_index].end());
	wstring filename = L"Texture/" + tmpName + L".dds";
	// 텍스처 정보가 없는경우 false
	ifstream in(filename);
	if (!in) return false;

	resourceTypes[_index] = _resourceType;

	pTextureBuffers[_index] = CreateTextureResourceFromDDSFile(_pDevice.Get(), _pCommandList.Get(), filename.c_str(), pTextureUploadBuffers[_index].GetAddressOf(), D3D12_RESOURCE_STATE_GENERIC_READ);

	return true;

}

void Texture::ReleaseUploadBuffers() {
	for (int i = 0; i < nTexture; i++) {
		if (pTextureUploadBuffers[i]) pTextureUploadBuffers[i].Reset();
	}
}

void Texture::SetSRVGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle)
{
	// 텍스처에 GPU 디스크립터 핸들을 저장한다.
	srvGpuDescriptorHandles[_index] = srvGpuDescriptorHandle;
}

void Texture::SetUAVGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE uavGpuDescriptorHandle)
{
	// 텍스처에 uav에 대한 GPU 디스크립터 핸들을 저장한다.
	uavGpuDescriptorHandles[_index] = uavGpuDescriptorHandle;
}

void Texture::SetSRVComputeGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle)
{
	// 텍스처에 GPU 디스크립터 핸들을 저장한다.
	srvComputeGpuDescriptorHandles[_index] = srvGpuDescriptorHandle;
}

void Texture::SetUAVComputeGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE uavGpuDescriptorHandle)
{
	// 텍스처에 uav에 대한 GPU 디스크립터 핸들을 저장한다.
	uavComputeGpuDescriptorHandles[_index] = uavGpuDescriptorHandle;

}


ComPtr<ID3D12Resource> Texture::CreateTexture(const ComPtr<ID3D12Device>& _pDevice, UINT _width, UINT _height, DXGI_FORMAT _format, D3D12_RESOURCE_FLAGS _resourceFlags, D3D12_RESOURCE_STATES _resourceStates, D3D12_CLEAR_VALUE* _clearValue, UINT _resourceType, UINT _index) {
	resourceTypes[_index] = _resourceType;
	pTextureBuffers[_index] = ::CreateTexture2DResource(_pDevice.Get(), _width, _height, 1, 0, _format, _resourceFlags, _resourceStates, _clearValue);
	return(pTextureBuffers[_index]);
}

void Texture::CreateBuffer(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex)
{
	resourceTypes[nIndex] = RESOURCE_BUFFER;
	bufferFormats[nIndex] = ndxgiFormat;
	bufferElements[nIndex] = nElements;
	bufferStrides[nIndex] = nStride;
	pTextureBuffers[nIndex] = ::CreateBufferResource(_pDevice, _pCommandList, pData, nElements * nStride, d3dHeapType, d3dResourceStates, pTextureUploadBuffers[nIndex]);
}

D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::GetUnorderedAccessViewDesc(int _index) {
	ComPtr< ID3D12Resource> pShaderResource = GetResource(_index);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc;

	int nTextureType = GetTextureType(_index);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;
		
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		d3dUnorderedAccessViewDesc.Texture2DArray.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		d3dUnorderedAccessViewDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dUnorderedAccessViewDesc.Format = bufferFormats[_index];
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		d3dUnorderedAccessViewDesc.Buffer.FirstElement = 0;
		d3dUnorderedAccessViewDesc.Buffer.NumElements = 0;
		d3dUnorderedAccessViewDesc.Buffer.StructureByteStride = 0;
		d3dUnorderedAccessViewDesc.Buffer.CounterOffsetInBytes = 0;
		d3dUnorderedAccessViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	}
	return(d3dUnorderedAccessViewDesc);
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetShaderResourceViewDesc(int _index)
{

	ComPtr<ID3D12Resource> pShaderResource = GetResource(_index);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(_index);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = bufferFormats[_index];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = bufferElements[_index];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}


/////////////////////////// TextureManager ////////////////////////


shared_ptr<Texture> TextureManager::GetTexture(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _rootParameterIndex) {
	if (!storage.contains(_name)) {	// 처음 불러온 텍스처일 경우
		GameFramework& gameFramework = GameFramework::Instance();
		shared_ptr<Texture> newTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);

		bool result = newTexture->LoadFromFile(_name, _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, _rootParameterIndex);

		if (!result) return nullptr;

		Shader::CreateShaderResourceViews(_pDevice, newTexture, 0, _rootParameterIndex);
		storage[_name] = newTexture;
	}
	return storage[_name];
}

shared_ptr<Texture> TextureManager::GetExistTexture(const string& _name)
{
	if (!storage.contains(_name)) {	// 처음 불러온 텍스처일 경우
		cout << "텍스처가 없습니다!!\n";
		return nullptr;
	}
	return storage[_name];
}

void TextureManager::ReleaseUploadBuffers() {
	for (auto& pTexture : storage) {
		pTexture.second->ReleaseUploadBuffers();
	}
}