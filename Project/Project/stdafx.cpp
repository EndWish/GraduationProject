#include "stdafx.h"

void ReadStringBinary(string& _dest, ifstream& _file){
	int len;
	_file.read((char*)&len, sizeof(len));
	_dest.assign(len, ' ');
	_file.read((char*)_dest.data(), sizeof(char) * len);
}

ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates) {
	ComPtr<ID3D12Resource> pBuffer;

	// ���� ���� ����
	D3D12_HEAP_PROPERTIES heapPropertiesDesc;
	::ZeroMemory(&heapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	heapPropertiesDesc.Type = _heapType;
	heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDesc.CreationNodeMask = 1;
	heapPropertiesDesc.VisibleNodeMask = 1;

	// ���ҽ��� ���� ����
	D3D12_RESOURCE_DESC resourceDesc;
	::ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = _byteSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ���ҽ��� ó�� ����
	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;	// ���ҽ��� ����Ʈ Ÿ���ϰ��
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {		// ���ҽ��� ���ε� Ÿ���ϰ��
		resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (_heapType == D3D12_HEAP_TYPE_READBACK) {	// ���ҽ��� ����� Ÿ���ϰ��
		resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	// ���� ������ ���ҽ� ������ ������ ���ҽ��� �����.
	HRESULT hResult = _pDevice->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, NULL, __uuidof(ID3D12Resource), (void**)&pBuffer);


	if (_pData) {
		if (_heapType == D3D12_HEAP_TYPE_DEFAULT) {
			// ����Ʈ ���ҽ��� ��� CPU�� ���� �����͸� ���� ���ϱ� ������ ���ε� ���۸� ���� ����Ʈ���ۿ� �����Ѵ�.

			ComPtr<ID3D12Resource> pUploadBuffer;

			// GPU�� ���ε� ���۸� �����(�Ҵ��Ѵ�).
			heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
			_pDevice->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)&pUploadBuffer);

			// ������ ���� ���ε������ �ּҰ��� �˾Ƴ��� CPU�޸� �� �ִ� ������(_pData)�� GPU�޸�(pUploadBuffer)�� �����Ѵ�.
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			pUploadBuffer->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			pUploadBuffer->Unmap(0, NULL);

			_pCommandList->CopyResource(pBuffer.Get(), pUploadBuffer.Get());

			D3D12_RESOURCE_BARRIER resourceBarrier;
			::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
			resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resourceBarrier.Transition.pResource = pBuffer.Get();
			resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			resourceBarrier.Transition.StateAfter = _resourceStates;
			resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			_pCommandList->ResourceBarrier(1, &resourceBarrier);
		}
		else if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			pBuffer->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			pBuffer->Unmap(0, NULL);
		}
		else if (_heapType == D3D12_HEAP_TYPE_READBACK) {

		}
	}

	return pBuffer;
}

//xmfloat ����ϱ�
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3) {
	os << "(" << f3.x << " " << f3.y << " " << f3.z << ")";
	return os;
}
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4) {
	os << "(" << f4.x << " " << f4.y << " " << f4.z << f4.w << ")";
	return os;
}
std::ostream& operator<<(std::ostream& os, const XMFLOAT4X4& f4x4) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			os << f4x4.m[i][j] << " ";
		}
		os << "\n";
	}
	return os;
}
