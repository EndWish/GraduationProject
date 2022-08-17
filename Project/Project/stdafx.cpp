#include "stdafx.h"

void ReadStringBinary(string& _dest, ifstream& _file){
	int len;
	_file.read((char*)&len, sizeof(len));
	_dest.assign(len, ' ');
	_file.read((char*)_dest.data(), sizeof(char) * len);
}

void CreateBufferResource(ComPtr<ID3D12Resource>* _dst, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates,ComPtr<ID3D12Resource>* _pUploadBuffer) {
	//ComPtr<ID3D12Resource> pBuffer;
	// IID_PPV_ARGS = __uidof(type), (void**)(ref)

	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;	// 리소스가 디폴트 타입일경우
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {		// 리소스가 업로드 타입일경우
		resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (_heapType == D3D12_HEAP_TYPE_READBACK) {	// 리소스가 리드백 타입일경우
		resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	cout << _pUploadBuffer;
	// 힙의 정보와 리소스 정보를 가지고 리소스를 만든다.
	auto heapType = CD3DX12_HEAP_PROPERTIES(_heapType);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_byteSize);
	HRESULT hResult = _pDevice->CreateCommittedResource(&heapType, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, nullptr, IID_PPV_ARGS((*_dst).GetAddressOf()));
	
	// 리소스의 처음 상태
	if (FAILED(hResult)) {
		cout << "기본 디폴트 힙 생성 오류!\n";
	}

	if (_pData) {
		if (_heapType == D3D12_HEAP_TYPE_DEFAULT) {
			if (_pUploadBuffer) {
				cout << _pUploadBuffer;
				// 디폴트 리소스의 경우 CPU가 직접 데이터를 쓰지 못하기 때문에 업로드 버퍼를 거쳐 디폴트버퍼에 복사한다.

			// GPU에 업로드 버퍼를 만든다(할당한다).
				auto upHeapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				auto upResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_byteSize);
				HRESULT hResult = _pDevice->CreateCommittedResource(&upHeapType, D3D12_HEAP_FLAG_NONE, &upResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS((* _pUploadBuffer).GetAddressOf()));
				// 리소스의 처음 상태
				if (FAILED(hResult)) {
					cout << "임시 힙 생성 오류!\n";
				}

				// 위에서 만든 업로드버퍼의 주소값을 알아내어 CPU메모리 에 있는 데이터(_pData)를 GPU메모리(pUploadBuffer)에 복사한다.
				D3D12_RANGE readRange = { 0, 0 };
				shared_ptr<UINT8> pBufferDataBegin;
				(*_pUploadBuffer)->Map(0, &readRange, (void**)&pBufferDataBegin);
				memcpy(pBufferDataBegin.get(), _pData, _byteSize);
				(*_pUploadBuffer)->Unmap(0, NULL);

				// 현재 이 함수가 제대로 작동하지 않는것 같다.
				_pCommandList->CopyResource((*_dst).Get(), (*_pUploadBuffer).Get());

				D3D12_RESOURCE_BARRIER resourceBarrier;
				::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				resourceBarrier.Transition.pResource = (*_dst).Get();
				resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				resourceBarrier.Transition.StateAfter = _resourceStates;
				resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				_pCommandList->ResourceBarrier(1, &resourceBarrier);
			}
		}
		else if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			(*_dst)->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			(*_dst)->Unmap(0, NULL);
		}
		else if (_heapType == D3D12_HEAP_TYPE_READBACK) {

		}
	}
}

//xmfloat 출력하기
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
