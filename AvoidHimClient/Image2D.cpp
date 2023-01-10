#include "stdafx.h"
#include "Image2D.h"
#include "GameFramework.h"

Image2D::Image2D(const string& _fileName, XMFLOAT2 _size, XMFLOAT2 _position, XMFLOAT2 _uvsize, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	GameFramework& gameFramework = GameFramework::Instance();
	enable = true;
	pTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);

	pTexture->LoadFromFile(_fileName, _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	cout << _fileName << "\n";
	position = XMFLOAT2(_position.x - 1, -_position.y + 1 - _size.y);
	startuv = XMFLOAT2(0, 0);
	sizeuv = _uvsize;
	pressed = false;
	array<XMFLOAT2, 6> vertex;

	vertex[0] = XMFLOAT2(0, 0);
	vertex[1] = XMFLOAT2(_size.x, 0);
	vertex[2] = XMFLOAT2(0, _size.y);
	vertex[3] = XMFLOAT2(_size.x, _size.y);
	vertex[4] = vertex[2];
	vertex[5] = vertex[1];


	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, vertex.data(), sizeof(XMFLOAT2) * 6, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT2);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT2) * 6;

	shared_ptr<Shader> pUIShader = gameFramework.GetShader("UIShader");
	pUIShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

Image2D::~Image2D() {

}

void Image2D::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	XMFLOAT4X4 world = Matrix4x4::Identity();
	world._11 = position.x;
	world._22 = position.y;

	world._14 = startuv.x;
	world._24 = sizeuv.x;
	world._34 = startuv.y;
	world._44 = sizeuv.y;

	if (pressed) world._41 = 1.0f;
	else world._41 = 0.0f;
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void Image2D::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (!enable) return;

	UpdateShaderVariable(_pCommandList);
	pTexture->UpdateShaderVariable(_pCommandList);

	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews = { positionBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, &vertexBuffersViews);
	_pCommandList->DrawInstanced(6, 1, 0, 0);
}