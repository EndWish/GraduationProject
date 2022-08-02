#include "stdafx.h"
#include "GameFramework.h"

/// 정적 변수▼
unique_ptr<GameFramework> GameFramework::s_pInstance;

/// 정적 함수▼
// 생성 함수 및 소멸 함수
bool GameFramework::Create(HINSTANCE hInstance, HWND hMainWnd) 
{
	if (!s_pInstance) {
		// GameGramwork 가 존재하지 않으면
		s_pInstance.reset(new GameFramework());	// 생성한다.
		GameFramework& gameFramework = *s_pInstance;

		gameFramework.m_hInstance = hInstance;
		gameFramework.m_hWnd = hMainWnd;

		gameFramework.CreateDirect3dDevice();	// 가장먼저 디바이스를 생성해야 명령 대기열이나 서술자 힙 등을 생성할 수 있다.
		gameFramework.CreateCommandQueueAndList();
		gameFramework.CreateRtvAndDsvDescriptorHeaps();
		gameFramework.CreateSwapChain();	// DxgiFactory, CommandQueue, RtvDescriptorHeap 이 미리 만들어져 있어야 한다.
		gameFramework.CreateDepthStencilView();
		gameFramework.CreateGraphicsRootSignature();
		
		Shader::instance.CreateShader(gameFramework.m_pDevice, gameFramework.m_pRootSignature);	// 임시로 쉐이더를 생성해봄 [수정]

		// 최초의 씬 빌드 [수정]
		shared_ptr<Scene> startScene = make_shared<Scene>();
		gameFramework.PushScene(startScene);

		gameFramework.m_gameTimer.Reset();	// 타이머 리셋

		return true;
	}

#ifdef DEBUG
	cout << "GameFramework::OnCreate() : 이미 생성된 GameFramework를 한번더 만들려고 하였습니다.\n";
#endif // DEBUG

	return false;
}
void GameFramework::Destroy() 
{
	if (s_pInstance) {
		GameFramework& gameFramework = *s_pInstance;

		// 씬들을 Destroy 한다[수정]
		::CloseHandle(gameFramework.m_fenceEvent);
	}

#ifdef DEBUG
	cout << "GameFramework::OnDestroy() : 생성된 GameFramework 가 없는데 소멸시키려고 하였습니다.\n";
#endif // DEBUG
}
GameFramework& GameFramework::Instance() 
{
#ifdef DEBUG
	if (!s_pInstance)
		cout << "GameFramework::Instance() : Create하지 않은 상태에서 호출하였습니다.\n";
#endif // DEBUG

	return *s_pInstance;
}

// 생성관련 멤버 함수▼
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

	//------------------------------------게임관련 변수-------------------------------------
	// 타이머
	m_gameTimer = GameTimer();

}
GameFramework::~GameFramework() 
{

}

// 제일 처음 생성시해야 하는 것들 (OnCreate()에 포함된다.)
void GameFramework::CreateDirect3dDevice() 
{

	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pDxgiFactory );	// m_pDxgiFactory.GetAddressOf()와 &m_pDxgiFactory 는 같다.

	ComPtr<IDXGIAdapter1> pAdapter = NULL;	// 알아서 소멸됨

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDxgiFactory->EnumAdapters1(i, &pAdapter); i++) {	// i번째 어뎁터(그래픽 카드)에 대한 포인터를 pD3dAdapter변수에 저장
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pAdapter->GetDesc1(&dxgiAdapterDesc);	// 어댑터의 정보를 가져온다.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // 어댑터가 소프트웨어이면 건너뛴다.
			continue;	

		if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pDevice)))	// 합당한 어댑터가 있으면 그것으로 디바이스를 생성한다.
			break;
	}

	if (!pAdapter) {
		m_pDxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)pAdapter.GetAddressOf());
		hResult = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pDevice );
	}

	// 다중샘플링 정보
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevels.SampleCount = 4;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_msaa4xQualityLevels = msaaQualityLevels.NumQualityLevels;
	m_msaa4xEnable = (m_msaa4xQualityLevels > 1) ? true : false;

	hResult = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_fenceValues[i] = 0;

	m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

}
void GameFramework::CreateCommandQueueAndList() 
{
	HRESULT hResult;

	// 명령 대기열 생성
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	::ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pDevice->CreateCommandQueue(&commandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pCommandQueue);

	// 명령 할당자 생성
	hResult = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pCommandAllocator);

	// 명령 목록 생성 - CPU를 통해 명령 목록을 명령 대기열에 전달했다고 해서 즉시 실행되는 것이 아님에 주의! (CPU와 GPU는 독립적으로 돌아가기 때문에)
	hResult = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pCommandList);
	hResult = m_pCommandList->Close();
	 
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
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	::ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	descriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// 현래 랜더 타켓은 전면, 후면 2개 사용
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pRtvDescriptorHeap);
	m_rtvDescriptorIncrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// 렌더 타겟 뷰를 만들때 사용됨

	// 깊이-스텐실의 서술자 힙 생성
	descriptorHeapDesc.NumDescriptors = 1;	// 깊이-스텐실은 1개 사용
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pDsvDescriptorHeap);
	m_dsvDescriptorIncrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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

	HRESULT hResult = m_pDxgiFactory->CreateSwapChain(m_pCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_pDxgiSwapChain.GetAddressOf() );

	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pDxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	CreateRenderTargetViews();	// RtvDescriptorHeap 이 미리 만들어져 있어야 한다.
}
void GameFramework::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = m_pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	// 서술자 힙을 통해 시작주소를 가져온다.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppRenderTargetBuffers[i]);	// 렌더타켓 버퍼를 생성한다.
		m_pDevice->CreateRenderTargetView(m_ppRenderTargetBuffers[i].Get(), NULL, rtvCPUDescriptorHandle);	// 렌더타켓 뷰를 서술자 힙에 생성(적제) (뷰==서술자?)
		rtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;	// 다음번 주소로 이동
	}
}
void GameFramework::CreateDepthStencilView()
{
	// 깊이-스텐실 버퍼(리소스)를 만들기 위한 정보들 입력
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = m_windowClientWidth;
	resourceDesc.Height = m_windowClientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties;
	::ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// 깊이-스텐실 버퍼(리소스) 생성
	m_pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&m_pDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	::ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, dsvCPUDescriptorHandle);	// 깊이-스텐실 뷰를 서술자 힙에 생성(적제) (뷰==서술자?)
}
void GameFramework::CreateGraphicsRootSignature() {
	D3D12_ROOT_PARAMETER pRootParameters[2];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pRootParameters[0].Descriptor.RegisterSpace = 0;
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[1].Constants.Num32BitValues = 32;
	pRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pRootParameters[1].Constants.RegisterSpace = 0;
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootSignatureDesc.NumParameters = _countof(pRootParameters);
	rootSignatureDesc.pParameters = pRootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = NULL;
	rootSignatureDesc.Flags = rootSignatureFlags;

	ID3DBlob* pSignatureBlob = NULL;
	ID3DBlob* pErrorBlob = NULL;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);
	m_pDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&m_pRootSignature);
	if (pSignatureBlob)
		pSignatureBlob->Release();
	if (pErrorBlob)
		pErrorBlob->Release();
}

/// 멤버 함수▼
// Get Set 함수
Scene& GameFramework::GetCurrentSceneRef()
{
#ifdef DEBUG
	if (m_pScenes.empty()) {
		cout << "GameFramework::GetCurrentScene() : 씬이 존재하지 않는데 씬을 얻을려고 함!\n";
	}
#endif // DEBUG

	return *m_pScenes.top();
}
pair<int, int> GameFramework::GetWindowClientSize() const 
{
	return make_pair(m_windowClientWidth, m_windowClientHeight);
}

// 다음 프레임으로 진행하는 함수
void GameFramework::FrameAdvance() 
{
	m_gameTimer.Tick(60.0f);

	// 입력을 처리한다. [수정]
	// 씬을 진행시킨다. [수정]
	
	
	//----------------------------------- 렌더링 한다 ----------------------------------
	//명령 할당자와 명령 리스트를 리셋한다. 
	HRESULT hResult = m_pCommandAllocator->Reset();
	hResult = m_pCommandList->Reset(m_pCommandAllocator.Get(), NULL);

	/*현재 렌더 타겟에 대한 프리젠트가 끝나기를 기다린다. 프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태
	(D3D12_RESOURCE_STATE_PRESENT)에서 렌더 타겟 상태(D3D12_RESOURCE_STATE_RENDER_TARGET)로 바
	뀔 것이다.*/
	D3D12_RESOURCE_BARRIER resourceBarrier;
	::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = m_ppRenderTargetBuffers[m_swapChainBufferCurrentIndex].Get();
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &resourceBarrier);

	//현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들)를 계산한다.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = m_pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCPUDescriptorHandle.ptr += (m_swapChainBufferCurrentIndex * m_rtvDescriptorIncrementSize);

	//깊이-스텐실 서술자의 CPU 주소를 계산한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//렌더 타겟 뷰(서술자)와 깊이-스텐실 뷰(서술자)를 출력-병합 단계(OM)에 연결한다. 
	m_pCommandList->OMSetRenderTargets(1, &rtvCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);

	//원하는 색상으로 렌더 타겟(뷰)을 지운다. 
	float pClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };	/*Colors::Azure*/
	m_pCommandList->ClearRenderTargetView(rtvCPUDescriptorHandle, pClearColor, 0, NULL);

	//원하는 값으로 깊이-스텐실(뷰)을 지운다. 
	m_pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//그래픽 루트 시그너쳐를 파이프라인에 연결(설정)한다.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	//씬 렌더링	(Scene에서 카메라, 플레이어를 관린한다.)
	//GetCurrentSceneRef().Render(m_pCommandList);	[수정]

	/*현재 렌더 타겟에 대한 렌더링이 끝나기를 기다린다. GPU가 렌더 타겟(버퍼)을 더 이상 사용하지 않으면 렌더 타겟
	의 상태는 프리젠트 상태(D3D12_RESOURCE_STATE_PRESENT)로 바뀔 것이다.*/
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &resourceBarrier);

	//명령 리스트를 닫힌 상태로 만든다. 
	hResult = m_pCommandList->Close();

	//명령 리스트를 명령 큐에 추가하여 실행한다. 
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다. 
	WaitForGpuComplete();

	/*스왑체인을 프리젠트한다. 프리젠트를 하면 현재 렌더 타겟(후면버퍼)의 내용이
	전면버퍼로 옮겨지고 렌더 타겟 인덱스가 바뀔 것이다.*/
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// 다음 프레임으로 이동
	MoveToNextFrame();

	// 메뉴바 이름을 FPS로 변경
	wstring ws = L"FPS : " + to_wstring(m_gameTimer.GetFPS());
	::SetWindowText(m_hWnd, (LPCWSTR)ws.c_str());	// FPS를 제한 하지 않을 경우 렉걸림

}
//GPU와 동기화을 위한 대기
void GameFramework::WaitForGpuComplete() 
{
	//CPU 펜스의 값을 증가시키고,
	//GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다. 
	UINT64 fenceValue = ++m_fenceValues[m_swapChainBufferCurrentIndex];
	HRESULT hResult = m_pCommandQueue->Signal(m_pFence.Get(), fenceValue);
	if (m_pFence->GetCompletedValue() < fenceValue) {
		//펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다. 
		hResult = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
// 다음 후면버퍼로 바꾸고 WaitForGpuComplete() 수행
void GameFramework::MoveToNextFrame() 
{
	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	WaitForGpuComplete();
}

// 씬 관련 함수들
void GameFramework::PushScene(const shared_ptr<Scene>& pScene)
{
	// 현재 씬을 일시정지
	if (!m_pScenes.empty())
		m_pScenes.top()->Pause();

	// 새로운 씬으로 전환
	m_pScenes.push(pScene);

	// 새로운 씬 초기화
	m_pScenes.top()->Enter();
	
}
void GameFramework::PopScene() 
{
#ifdef DEBUG
	if (m_pScenes.empty()) {
		cout << "GameFramework::PopScene() : 씬이 존재하지 않는데 씬을 삭제하려고 함!\n";
	}
#endif // DEBUG

	m_pScenes.top()->Exit();
	m_pScenes.pop();
}
void GameFramework::ChangeScene(const shared_ptr<Scene>& pScene)
{
	PopScene();
	PushScene(pScene);
}
void GameFramework::ClearScene()
{
	while (!m_pScenes.empty())
		PopScene();
}
