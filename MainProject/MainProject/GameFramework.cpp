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

	CreateDirect3dDevice();	// ������� ����̽��� �����ؾ� ��� ��⿭�̳� ������ �� ���� ������ �� �ִ�.
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();	// DxgiFactory, CommandQueue, RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
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
void GameFramework::CreateCommandQueueAndList() 
{
	HRESULT hResult;

	// ��� ��⿭ ����
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pD3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pD3dCommandQueue);

	// ��� �Ҵ��� ����
	hResult = m_pD3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pD3dCommandAllocator);

	// ��� ��� ���� - CPU�� ���� ��� ����� ��� ��⿭�� �����ߴٰ� �ؼ� ��� ����Ǵ� ���� �ƴԿ� ����! (CPU�� GPU�� ���������� ���ư��� ������)
	hResult = m_pD3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pD3dCommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pD3dCommandList);
	hResult = m_pD3dCommandList->Close();
	 
	/*	���� ���� : ��� ��⿭CommandQueue�� ExecuteCommandLists �Լ��� ���ظ�� ���(CommandList)�� ���� �� �ִ�.
		��ɵ��� ��� ��Ͽ� �� �߰�������, Close �޼��带 ȣ���ؼ� ��ɵ��� ����� �������� Direct3D�� �˷���� �Ѵ�.
		������ (CPU�� ����) ��� ��⿭�� ��� ����� �־��ٰ� �ؼ� �ٷ� ����Ǵ� ���� �ƴϴ�. (CPU�� GPU�� ���������� ����Ǳ� ������) 
		
		D3D12_COMMAND_LIST_TYPE_DIRECT : GPU�� ���� ������ �� �ִ� ��� ����
		https://lipcoder.tistory.com/41
	*/

}
void GameFramework::CreateRtvAndDsvDescriptorHeaps() 
{

	// ���� Ÿ���� ������ �� ����
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// ���� ���� Ÿ���� ����, �ĸ� 2�� ���
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pD3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pD3dRtvDescriptorHeap);
	m_rtvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// ���� Ÿ�� �並 ���鶧 ����

	// ����-���ٽ��� ������ �� ����
	d3dDescriptorHeapDesc.NumDescriptors = 1;	// ����-���ٽ��� 1�� ���
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pD3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pD3dDsvDescriptorHeap);
	m_dsvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}
void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);	// Ŭ���̾�Ʈ ũ�⸦ rcClient�� ����
	m_windowClientWidth = rcClient.right - rcClient.left;
	m_windowClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;	// ����ü��(���� Ÿ��) ������ ����
	dxgiSwapChainDesc.BufferDesc.Width = m_windowClientWidth;	// ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Height = m_windowClientHeight;	// ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// �ȼ��� RGBA�� 8bit ���·� �׸���
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	// RefreshRate : ȭ�� ���ŷ�(������) //����
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	// �и�
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ��¿� ���۷� ���ڴٰ� �����ϴ°�.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// ����ü���� �⺻ ����.
	dxgiSwapChainDesc.OutputWindow = m_hWnd;	// ����� ������
	dxgiSwapChainDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;	// â��带 ������?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pDxgiFactory->CreateSwapChain(m_pD3dCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_pDxgiSwapChain.GetAddressOf() );

	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pDxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	CreateRenderTargetViews();	// RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
}
void GameFramework::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pD3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	// ������ ���� ���� �����ּҸ� �����´�.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppD3dRenderTargetBuffers[i]);	// ����Ÿ�� ���۸� �����Ѵ�.
		m_pD3dDevice->CreateRenderTargetView(m_ppD3dRenderTargetBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);	// ����Ÿ�� �並 ������ ���� ����(����) (��==������?)
		d3dRtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;	// ������ �ּҷ� �̵�
	}
}
