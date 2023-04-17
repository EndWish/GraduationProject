#pragma once
#include "GameObject.h"

class TextBox  {

private:
	WCHAR text[20];	// 텍스트 내용
	D2D1_RECT_F  rect; // 범위
	XMFLOAT2 size;
	ComPtr<IDWriteTextFormat> format;	// 텍스트 포맷
	ComPtr<ID2D1SolidColorBrush> brush;

	bool enable;
public:
public:
	TextBox(WCHAR* _fontName, D2D1::ColorF _color, XMFLOAT2 _position, XMFLOAT2 _size, float _fontSize, bool _enable = true);
	~TextBox();

	void SetText(const WCHAR _text[20]);

	const WCHAR* GetText() const { return text; };

	void SetEnable(bool _enable) { enable = _enable; };
	bool GetEnable() const { return enable; };

	void SetPosition(XMFLOAT2 _position, bool _isCenterBasis = false);

	void Render();
	void SetAlignment(DWRITE_TEXT_ALIGNMENT _alignment);

};



class TextLayer {
private:
	static unique_ptr<TextLayer> spInstance;
public:
	static void Create(UINT _nFrames, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12CommandQueue>& _pCommandQueue, array<ComPtr<ID3D12Resource>, 2> _renderTargets, UINT _nWidth, UINT _nHeight);
	static TextLayer& Instance();

private:
	float                           fWidth;
	float                           fHeight;
	
	ComPtr<IDWriteFontCollection1> fonts;

	ComPtr<ID3D11DeviceContext> pD3d11DeviceContext = NULL;
	ComPtr<ID3D11On12Device> pD3d11On12Device = NULL;
	ComPtr<IDWriteFactory5> pWriteFactory = NULL;
	ComPtr<ID2D1Factory3> pD2dFactory = NULL;
	ComPtr<ID2D1Device2> pD2dDevice = NULL;
	ComPtr<ID2D1DeviceContext2> pD2dDeviceContext = NULL;

	UINT nRenderTargets = 0;
	vector<ComPtr<ID3D11Resource>> pD3d11WrappedRenderTargets;
	vector< ComPtr< ID2D1Bitmap1>> pD2dRenderTargets;
public:
	TextLayer();
	~TextLayer();

	void LoadFont();
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12CommandQueue>& _pCommandQueue, array<ComPtr<ID3D12Resource>, 2> _renderTargets);
	ComPtr<ID2D1SolidColorBrush> CreateBrush(D2D1::ColorF _color);
	ComPtr<IDWriteTextFormat> CreateTextFormat(WCHAR* _fontName, float _fontSize);	
	
	// Get 함수
	ComPtr<ID3D11DeviceContext> GetD3D11DeviceContext() { return pD3d11DeviceContext; };
	ComPtr< ID3D11On12Device> GetD3D11On12Device() { return pD3d11On12Device; };
	ComPtr<ID2D1DeviceContext2> GetD2DDeviceContext() { return pD2dDeviceContext; };
	vector<ComPtr<ID3D11Resource>> GetWrappedRenderTargets() { return pD3d11WrappedRenderTargets; };
	vector< ComPtr< ID2D1Bitmap1>> GetRenderTargets() {	return pD2dRenderTargets; };
};

class Image2D {
private:
	// -1 ~ 1
	string name;

	bool pressed;	// 버튼에 있는 이미지가 눌렸을 경우
	bool dark;	// 일부 이미지의 경우 회색으로 그릴 bool값
	bool enable;	// 해당 이미지가 그려지는지
	XMFLOAT2 startuv;	// 이미지의 시작 uv좌표
	XMFLOAT2 size;
	XMFLOAT2 sizeuv;	// 이미지의 크기
	XMFLOAT2 position;

	shared_ptr<Texture> pTexture;

	// position 내의 x,z값을 사용
	// cbuffer에서의 worldT._11, 22 = 시작점의 위치
	// _41, _42, _43, _44 = uv.x의 시작점, 크기, uv.y의 시작점, 크기
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

public:

	Image2D(const string& _fileName, XMFLOAT2 _size, XMFLOAT2 _position, XMFLOAT2 _uvsize, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable = true);
	~Image2D();
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void ReleaseUploadBuffers() { pPositionUploadBuffer.Reset(); };
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void SetEnable(bool _enable) { enable = _enable; };
	void SetStartUV(XMFLOAT2 _startuv) { startuv = _startuv; };
	void SetPosition(XMFLOAT2 _position) { position = XMFLOAT2(_position.x - 1, -_position.y + 1 - size.y); };
	void SetSizeUV(XMFLOAT2 _sizeuv) { sizeuv = _sizeuv; };
	void SetPress(bool _isClick) { pressed = _isClick; };
	void SetDark(bool _isDark) { dark = _isDark; };

	void SetTexture(const string& _name);
};

/////////////////////////////////////