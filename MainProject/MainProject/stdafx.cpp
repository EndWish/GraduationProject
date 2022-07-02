#include "stdafx.h"

ID3D12Resource* CreateBufferResource(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceStates, ID3D12Resource** ppUploadBuffer) 
{
	ID3D12Resource* pBuffer = NULL;

	D3D12_HEAP_PROPERTIES heapPropertiesDesc;
	::ZeroMemory(&heapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	heapPropertiesDesc.Type = heapType;
	heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDesc.CreationNodeMask = 1;
	heapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	::ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = nBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	else if (heapType == D3D12_HEAP_TYPE_READBACK) 
		resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;

	HRESULT hResult = pDevice->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, NULL, __uuidof(ID3D12Resource), (void**)&pBuffer);

	if (pData) {
		switch (heapType) {
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			if (ppUploadBuffer) {
				heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				pDevice->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppUploadBuffer);

				D3D12_RANGE d3dReadRange = { 0, 0 };
				UINT8* pBufferDataBegin = NULL;
				(*ppUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
				memcpy(pBufferDataBegin, pData, nBytes);
				(*ppUploadBuffer)->Unmap(0, NULL);

				pCommandList->CopyResource(pBuffer, *ppUploadBuffer);

				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				d3dResourceBarrier.Transition.pResource = pBuffer;
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				d3dResourceBarrier.Transition.StateAfter = resourceStates;
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pCommandList->ResourceBarrier(1, &d3dResourceBarrier);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pBuffer->Unmap(0, NULL);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
			break;
		}
	}
	return pBuffer;
}
