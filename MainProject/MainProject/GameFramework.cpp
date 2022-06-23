#include "stdafx.h"
#include "GameFramework.h"

// 생성자 및 소멸자
GameFramework::GameFramework() 
{
	// 윈도우
	m_hInstance = NULL;
	m_hWnd = NULL;

	// 클라이언트 해상도
	m_windowClientWidth = FRAME_BUFFER_WIDTH;
	m_windowClientHeight = FRAME_BUFFER_HEIGHT;

	// MSAA 다중 샘플링
	m_msaa4xEnable = false;
	m_msaa4xQualityLevels = 0;

	// 현재 스왑체인의 후면 버퍼의 인덱스
	m_swapChainBufferCurrentIndex = 0;

	// 렌더 타켓, 깊이-스텐실의 서술자 원소의 크기
	m_rtvDescriptorIncrementSize = 0;
	m_dsvDescriptorIncrementSize = 0;

	// 펜스
	m_fenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) 
		m_fenceValues[i] = 0;

}
GameFramework::~GameFramework() 
{

}

// 생성시(초기화) 함수
bool GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd) {
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3dDevice();
	//CreateCommandQueueAndList();
	//CreateRtvAndDsvDescriptorHeaps();
	//CreateSwapChain();
	//CreateDepthStencilView();

	return(true);
}

// 제일 처음 생성시해야 하는 것들 (OnCreate()에 포함된다.)
void GameFramework::CreateDirect3dDevice() 
{

	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pDxgiFactory );	// m_pDxgiFactory.GetAddressOf()와 &m_pDxgiFactory 는 같다.

	ComPtr<IDXGIAdapter1> pD3dAdapter = NULL;	// 알아서 소멸됨

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDxgiFactory->EnumAdapters1(i, &pD3dAdapter); i++) {	// i번째 어뎁터(그래픽 카드)에 대한 포인터를 pD3dAdapter변수에 저장
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pD3dAdapter->GetDesc1(&dxgiAdapterDesc);	// 어댑터의 정보를 가져온다.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // 어댑터가 소프트웨어이면 건너뛴다.
			continue;	

		if (SUCCEEDED(D3D12CreateDevice(pD3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pD3dDevice)))	// 합당한 어댑터가 있으면 그것으로 디바이스를 생성한다.
			break;
	}

	if (!pD3dAdapter) {
		m_pDxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)pD3dAdapter.GetAddressOf());
		hResult = D3D12CreateDevice(pD3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pD3dDevice );
	}

	// 다중샘플링 정보
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pD3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_msaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_msaa4xEnable = (m_msaa4xQualityLevels > 1) ? true : false;

	hResult = m_pD3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pD3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_fenceValues[i] = 0;

	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

}
