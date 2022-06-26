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

// 생성시(초기화) 함수 및 소멸시 함수
bool GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd) {
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3dDevice();	// 가장먼저 디바이스를 생성해야 명령 대기열이나 서술자 힙 등을 생성할 수 있다.
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();	// DxgiFactory, CommandQueue, RtvDescriptorHeap 이 미리 만들어져 있어야 한다.
	CreateDepthStencilView();

	// 최초의 씬 빌드 [추가]
	m_gameTimer.Reset();

	return true;
}
void GameFramework::OnDestroy()
{
	// 씬들을 Destroy 한다.

	::CloseHandle(m_fenceEvent);
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
void GameFramework::CreateCommandQueueAndList() 
{
	HRESULT hResult;

	// 명령 대기열 생성
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pD3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pD3dCommandQueue);

	// 명령 할당자 생성
	hResult = m_pD3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pD3dCommandAllocator);

	// 명령 목록 생성 - CPU를 통해 명령 목록을 명령 대기열에 전달했다고 해서 즉시 실행되는 것이 아님에 주의! (CPU와 GPU는 독립적으로 돌아가기 때문에)
	hResult = m_pD3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pD3dCommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pD3dCommandList);
	hResult = m_pD3dCommandList->Close();
	 
	/*	관련 설명 : 명령 대기열CommandQueue의 ExecuteCommandLists 함수를 통해명령 목록(CommandList)을 넣을 수 있다.
		명령들을 명령 목록에 다 추가했으면, Close 메서드를 호출해서 명령들의 기록이 끝났음을 Direct3D에 알려줘야 한다.
		하지만 (CPU를 통해) 명령 대기열에 명령 목록을 넣었다고 해서 바로 실행되는 것은 아니다. (CPU와 GPU가 독립적으로 수행되기 때문에) 
		
		D3D12_COMMAND_LIST_TYPE_DIRECT : GPU가 직접 실행할 수 있는 명령 버퍼
		https://lipcoder.tistory.com/41
	*/

}
void GameFramework::CreateRtvAndDsvDescriptorHeaps() 
{

	// 랜더 타켓의 서술자 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// 현래 랜더 타켓은 전면, 후면 2개 사용
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pD3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pD3dRtvDescriptorHeap);
	m_rtvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// 렌더 타겟 뷰를 만들때 사용됨

	// 깊이-스텐실의 서술자 힙 생성
	d3dDescriptorHeapDesc.NumDescriptors = 1;	// 깊이-스텐실은 1개 사용
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pD3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pD3dDsvDescriptorHeap);
	m_dsvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}
void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);	// 클라이언트 크기를 rcClient에 저장
	m_windowClientWidth = rcClient.right - rcClient.left;
	m_windowClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;	// 스왑체인(렌더 타켓) 버퍼의 개수
	dxgiSwapChainDesc.BufferDesc.Width = m_windowClientWidth;	// 윈도우 클라이언트 영역의 가로 크기 
	dxgiSwapChainDesc.BufferDesc.Height = m_windowClientHeight;	// 윈도우 클라이언트 영역의 세로 크기 
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 픽셀을 RGBA에 8bit 형태로 그리기
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	// RefreshRate : 화면 갱신률(프레임) //분자
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	// 분모
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 출력용 버퍼로 쓰겠다고 지정하는것.		읽기전용, 쉐이더의 입력으로 사용, 공유, 무순서화 접근 등의 형태가 있다.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 전면 버퍼와 후면버퍼를 교체하고 버퍼 내용을 폐기(유지 하지 않으면 속도가 빠라짐)한다.
	dxgiSwapChainDesc.OutputWindow = m_hWnd;	// 출력할 윈도우
	dxgiSwapChainDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;	// 창모드를 쓸껀지?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pDxgiFactory->CreateSwapChain(m_pD3dCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_pDxgiSwapChain.GetAddressOf() );

	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pDxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	CreateRenderTargetViews();	// RtvDescriptorHeap 이 미리 만들어져 있어야 한다.
}
void GameFramework::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pD3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	// 서술자 힙을 통해 시작주소를 가져온다.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppD3dRenderTargetBuffers[i]);	// 렌더타켓 버퍼를 생성한다.
		m_pD3dDevice->CreateRenderTargetView(m_ppD3dRenderTargetBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);	// 렌더타켓 뷰를 서술자 힙에 생성(적제) (뷰==서술자?)
		d3dRtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;	// 다음번 주소로 이동
	}
}
void GameFramework::CreateDepthStencilView()
{
	// 깊이-스텐실 버퍼(리소스)를 만들기 위한 정보들 입력
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_windowClientWidth;
	d3dResourceDesc.Height = m_windowClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	// 깊이-스텐실 버퍼(리소스) 생성
	m_pD3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pD3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pD3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pD3dDevice->CreateDepthStencilView(m_pD3dDepthStencilBuffer.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);	// 깊이-스텐실 뷰를 서술자 힙에 생성(적제) (뷰==서술자?)
}

// 다음 프레임으로 진행하는 함수
void GameFramework::FrameAdvance() 
{
	m_gameTimer.Tick(60.0f);

	wstring ws = L"FPS : " + to_wstring(m_gameTimer.GetFPS());
	::SetWindowText(m_hWnd, (LPCWSTR)ws.c_str());	// 렉걸림
}
