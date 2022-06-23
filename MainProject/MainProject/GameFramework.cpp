#include "stdafx.h"
#include "GameFramework.h"

// ������ �� �Ҹ���
GameFramework::GameFramework() 
{
	// ������
	m_hInstance = NULL;
	m_hWnd = NULL;

	// Ŭ���̾�Ʈ �ػ�
	m_windowClientWidth = FRAME_BUFFER_WIDTH;
	m_windowClientHeight = FRAME_BUFFER_HEIGHT;

	// MSAA ���� ���ø�
	m_msaa4xEnable = false;
	m_msaa4xQualityLevels = 0;

	// ���� ����ü���� �ĸ� ������ �ε���
	m_swapChainBufferCurrentIndex = 0;

	// ���� Ÿ��, ����-���ٽ��� ������ ������ ũ��
	m_rtvDescriptorIncrementSize = 0;
	m_dsvDescriptorIncrementSize = 0;

	// �潺
	m_fenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) 
		m_fenceValues[i] = 0;

}
GameFramework::~GameFramework() 
{

}

// ������(�ʱ�ȭ) �Լ�
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

// ���� ó�� �������ؾ� �ϴ� �͵� (OnCreate()�� ���Եȴ�.)
void GameFramework::CreateDirect3dDevice() 
{

	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pDxgiFactory );	// m_pDxgiFactory.GetAddressOf()�� &m_pDxgiFactory �� ����.

	ComPtr<IDXGIAdapter1> pD3dAdapter = NULL;	// �˾Ƽ� �Ҹ��

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDxgiFactory->EnumAdapters1(i, &pD3dAdapter); i++) {	// i��° ���(�׷��� ī��)�� ���� �����͸� pD3dAdapter������ ����
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pD3dAdapter->GetDesc1(&dxgiAdapterDesc);	// ������� ������ �����´�.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // ����Ͱ� ����Ʈ�����̸� �ǳʶڴ�.
			continue;	

		if (SUCCEEDED(D3D12CreateDevice(pD3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pD3dDevice)))	// �մ��� ����Ͱ� ������ �װ����� ����̽��� �����Ѵ�.
			break;
	}

	if (!pD3dAdapter) {
		m_pDxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)pD3dAdapter.GetAddressOf());
		hResult = D3D12CreateDevice(pD3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pD3dDevice );
	}

	// ���߻��ø� ����
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
