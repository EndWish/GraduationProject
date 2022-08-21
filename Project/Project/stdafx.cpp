#include "stdafx.h"

void ReadStringBinary(string& _dest, ifstream& _file){
	int len;
	_file.read((char*)&len, sizeof(len));
	_dest.assign(len, ' ');
	_file.read((char*)_dest.data(), sizeof(char) * len);
	cout << "��Ʈ��:" << len << " " << _dest << "���̴�\n";
}
ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates, ComPtr<ID3D12Resource>& _pUploadBuffer) {
	ComPtr<ID3D12Resource> pBuffer;



	// ���ҽ��� ó�� ����
	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;	// ���ҽ��� ����Ʈ Ÿ���ϰ��
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {		// ���ҽ��� ���ε� Ÿ���ϰ��
		resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (_heapType == D3D12_HEAP_TYPE_READBACK) {	// ���ҽ��� ����� Ÿ���ϰ��
		resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	// ���� ���� ����
	auto heapType = CD3DX12_HEAP_PROPERTIES(_heapType);

	// ���ҽ��� ���� ����
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_byteSize);

	// ���� ������ ���ҽ� ������ ������ ���ҽ��� �����.
	HRESULT hResult = _pDevice->CreateCommittedResource(&heapType, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, NULL, IID_PPV_ARGS(&pBuffer));

	if (_pData) {
		if (_heapType == D3D12_HEAP_TYPE_DEFAULT) {
			// ����Ʈ ���ҽ��� ��� CPU�� ���� �����͸� ���� ���ϱ� ������ ���ε� ���۸� ���� ����Ʈ���ۿ� �����Ѵ�.

			// GPU�� ���ε� ���۸� �����(�Ҵ��Ѵ�).
			heapType.Type = D3D12_HEAP_TYPE_UPLOAD;
			hResult = _pDevice->CreateCommittedResource(&heapType, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&_pUploadBuffer));

			// ������ ���� ���ε������ �ּҰ��� �˾Ƴ��� CPU�޸� �� �ִ� ������(_pData)�� GPU�޸�(pUploadBuffer)�� �����Ѵ�.
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			_pUploadBuffer->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			_pUploadBuffer->Unmap(0, NULL);

			_pCommandList->CopyResource(pBuffer.Get(), _pUploadBuffer.Get());

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
	os << "(" << f4.x << " " << f4.y << " " << f4.z << " " << f4.w << ")";
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
