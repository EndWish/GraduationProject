#include "stdafx.h"
#include "Image2D.h"
#include "GameFramework.h"

unique_ptr<TextLayer> TextLayer::spInstance;

Image2D::Image2D(const string& _fileName, XMFLOAT2 _size, XMFLOAT2 _position, XMFLOAT2 _uvsize, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable)
{

	GameFramework& gameFramework = GameFramework::Instance();
	auto pShader = gameFramework.GetShader("UIShader");
	enable = _enable;
	pTexture = gameFramework.GetTextureManager().GetTexture(_fileName, _pDevice,  _pCommandList);
	
	name = _fileName;
	size = _size;
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

	// 이미지의 위치, 크기 등을 루트시그니처를 통해 보낸다.
	UpdateShaderVariable(_pCommandList);

	// 텍스처를 연결한다.
	pTexture->UpdateShaderVariable(_pCommandList);

	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews = { positionBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, &vertexBuffersViews);
	_pCommandList->DrawInstanced(6, 1, 0, 0);
}


void Image2D::SetTexture(const string& _name) {
	GameFramework& gameFramework = GameFramework::Instance();
	pTexture = gameFramework.GetTextureManager().GetExistTexture(_name);
}


TextBox::TextBox(WCHAR* _fontName, D2D1::ColorF _color, XMFLOAT2 _position, XMFLOAT2 _size, float _fontSize, bool _enable) {
	TextLayer& textLayer = TextLayer::Instance();

	D2D1_RECT_F rc{ _position.x * C_WIDTH / 2, _position.y * C_HEIGHT / 2, _position.x * C_WIDTH / 2 + _size.x * C_WIDTH / 2, _position.y * C_HEIGHT / 2 + _size.y * C_WIDTH / 2 };
	rect = rc;
	brush = textLayer.CreateBrush(_color);
	format = textLayer.CreateTextFormat(_fontName, _fontSize);
	enable = _enable;
}

TextBox::~TextBox() {
	
}

void TextBox::SetText(wstring _text) {
	text = _text;
}

void TextBox::Render()
{
	if (!enable) return;

	TextLayer& textLayer = TextLayer::Instance();
	GameFramework& gameFramework = GameFramework::Instance();

	// 현재 스왑체인의 인덱스
	UINT currIndex = gameFramework.GetCurrentSwapChainIndex();

	ComPtr<ID3D11DeviceContext> pD3D11DeviceContext = textLayer.GetD3D11DeviceContext();
	ComPtr<ID3D11On12Device> pD3D11On12Device = textLayer.GetD3D11On12Device();
	ComPtr<ID2D1DeviceContext2> pD2DDeviceContext = textLayer.GetD2DDeviceContext();

	ID3D11Resource* ppResources[] = { (textLayer.GetWrappedRenderTargets())[currIndex].Get() };

	pD2DDeviceContext->SetTarget(textLayer.GetRenderTargets()[currIndex].Get());
	pD3D11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

	pD2DDeviceContext->BeginDraw();
	pD2DDeviceContext->DrawText(text.c_str(), (UINT32)text.size(), format.Get(), rect, brush.Get());
	pD2DDeviceContext->EndDraw();
	pD3D11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
	pD3D11DeviceContext->Flush();

}


TextLayer::TextLayer() {
	fWidth = 0.f;
	fHeight = 0.f;
}

void TextLayer::Create(UINT _nFrames, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12CommandQueue>& _pCommandQueue, array<ComPtr<ID3D12Resource>, 2> _renderTargets, UINT _nWidth, UINT _nHeight)
{
	if (!spInstance) {
		spInstance.reset(new TextLayer());
		TextLayer& TextLayer = *spInstance;

		TextLayer.fWidth = static_cast<float>(_nWidth);
		TextLayer.fHeight = static_cast<float>(_nHeight);
		TextLayer.nRenderTargets = _nFrames;
		TextLayer.pD3d11WrappedRenderTargets.resize(_nFrames);
		TextLayer.pD2dRenderTargets.resize(_nFrames);

		TextLayer.Init(_pDevice, _pCommandQueue, _renderTargets);

	}
}

TextLayer& TextLayer::Instance()
{
	return *spInstance;
}

TextLayer::~TextLayer()
{

}

ComPtr<ID2D1SolidColorBrush> TextLayer::CreateBrush(D2D1::ColorF _color)
{
	ComPtr<ID2D1SolidColorBrush> colorBrush;
	pD2dDeviceContext->CreateSolidColorBrush(_color, colorBrush.GetAddressOf());
	return colorBrush;

}

ComPtr<IDWriteTextFormat> TextLayer::CreateTextFormat(WCHAR* _fontName, float _fontSize)
{
	ComPtr<IDWriteTextFormat> textFormat;
	pWriteFactory->CreateTextFormat(_fontName, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, _fontSize, L"en-us", &textFormat);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	return textFormat;
}

void TextLayer::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12CommandQueue>& _pCommandQueue, array<ComPtr<ID3D12Resource>, 2> _renderTargets)
{
	HRESULT hr;
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };
	ID3D11Device* pd3d11Device = NULL;
	ID3D12CommandQueue* ppd3dCommandQueues[] = { _pCommandQueue.Get() };
	hr = ::D3D11On12CreateDevice(_pDevice.Get(), d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, (ID3D11Device**)&pd3d11Device, (ID3D11DeviceContext**)pD3d11DeviceContext.GetAddressOf(), nullptr);
	//  ID3D11On12Device 객체 생성

	hr = pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&pD3d11On12Device);
	pd3d11Device->Release();	

	IDXGIDevice* pdxgiDevice = NULL;
	hr = pD3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);

	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&pD2dFactory);
	if(pdxgiDevice)
		hr = pD2dFactory->CreateDevice(pdxgiDevice, (ID2D1Device2**)&pD2dDevice);
	hr = pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2**)&pD2dDeviceContext);

	pD2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&pWriteFactory);
	pdxgiDevice->Release();

	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	for (UINT i = 0; i < nRenderTargets; i++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		pD3d11On12Device->CreateWrappedResource(_renderTargets[i].Get(), &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&pD3d11WrappedRenderTargets[i]));
		IDXGISurface* pdxgiSurface = NULL;
		pD3d11WrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
		if(pdxgiSurface)
			pD2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &pD2dRenderTargets[i]);
		pdxgiSurface->Release();
	}
}

