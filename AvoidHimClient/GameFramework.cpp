#include "stdafx.h"
#include "GameFramework.h"

unique_ptr<GameFramework> GameFramework::spInstance;


void GameFramework::Create(HINSTANCE _hInstance, HWND _hMainWnd) {

	if (!spInstance) {		// 프레임 워크 인스턴스가 생성된적이 없을 경우 
		spInstance.reset(new GameFramework());
		GameFramework& gameFramework = *spInstance;

		gameFramework.instanceHandle = _hInstance;
		gameFramework.windowHandle = _hMainWnd;
		
		gameFramework.CreateDirect3DDevice();    // 가장먼저 디바이스를 생성해야 명령 대기열이나 서술자 힙 등을 생성할 수 있다.
		gameFramework.CreateCommandQueueList();
		gameFramework.CreateRtvAndDsvDescriptorHeaps();
		gameFramework.CreateSwapChain();    // DxgiFactory, CommandQueue, RtvDescriptorHeap 이 미리 만들어져 있어야 한다.
		gameFramework.CreateRenderTargetViews();
		gameFramework.CreateDepthStencilView();
		gameFramework.CreateGraphicsRootSignature();
		gameFramework.CreateComputeRootSignature();

		CoInitialize(NULL);
		gameFramework.pCommandList->Reset(gameFramework.pCommandAllocator.Get(), NULL);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = gameFramework.pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (gameFramework.rtvDescriptorIncrementSize * gameFramework.nSwapChainBuffer);

		gameFramework.pSoundManager.Init(_hMainWnd);

		// 쉐이더 생성
		if (!gameFramework.InitShader()) {
			cout << "쉐이더 생성 실패\n";
		}
		// 쉐이더에 사용될 변수(경과시간 등)들 할당.
		gameFramework.CreateShaderVariables();

		// cbv, srv를 담기 위한 정적 디스크립터 힙 생성
		Shader::CreateCbvSrvUavDescriptorHeaps(gameFramework.pDevice, 0, 300, 1);
		Shader::CreateComputeDescriptorHeaps(gameFramework.pDevice, 0, 3, 1);



		// 텍스처 출력을 위한 TextLayer 인스턴스 초기화
		TextLayer::Create(nSwapChainBuffer, gameFramework.pDevice, gameFramework.pCommandQueue, gameFramework.pRenderTargetBuffers, gameFramework.clientWidth, gameFramework.clientHeight);
		
		// 디버그용 히트박스 메쉬 생성
		gameFramework.hitBoxMesh.Create(gameFramework.pDevice, gameFramework.pCommandList);
		
		// G Buffer 생성
		gameFramework.InitBuffer();

		// 파티클을 그리기 위한 리소스들 생성
		Shader::InitParticleResource(gameFramework.pDevice, gameFramework.pCommandList);

		// 최초씬 생성
		shared_ptr<Scene> pScene = make_shared<LobbyScene>();
		gameFramework.LoadingScene(pScene);
		gameFramework.PushScene(pScene);




		// 히트박스용 메쉬 생성
		//gameFramework.meshManager.GetHitBoxMesh().Create(gameFramework.pDevice, gameFramework.pCommandList);

		gameFramework.pCommandList->Close();

		vector<ComPtr<ID3D12CommandList>> pCommandLists = { gameFramework.pCommandList.Get() };
		gameFramework.pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());


		gameFramework.WaitForGpuComplete();
		//startScene->ReleaseUploadBuffers();
		gameFramework.gameTimer.Reset();    // 타이머 리셋
	}

}

void GameFramework::Destroy() {

	if (spInstance) {
		GameFramework& gameFramework = *spInstance;

		CloseHandle(gameFramework.fenceEvent);
	}
}

GameFramework& GameFramework::Instance() {
	return *spInstance;
}

void GameFramework::ProcessMouseInput(UINT _type, XMFLOAT2 _pos)
{

	switch (_type) {
	case WM_MOUSEMOVE:
		break;

	case WM_LBUTTONDOWN:

		if (isClick) return; // 처음 한번 마우스를 클릭 했을때만 수행
		break;
	case WM_LBUTTONUP:
		if (!isClick) return;
		break;
	}

	isClick = !isClick;	// 클릭 상태를 바꾸어준다.
	if (pScenes.top()) pScenes.top()->ProcessMouseInput(_type, _pos);
}

void GameFramework::ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	/// wParam : 소켓, lParam : select의 필드

	if (WSAGETSELECTERROR(_lParam)) {
		SockErrorDisplay(WSAGETSELECTERROR(_lParam));
		return;
	}
	// 메시지 처리
	switch (WSAGETSELECTEVENT(_lParam)) {
	case FD_READ:
		if (pScenes.top()) pScenes.top()->ProcessSocketMessage(pDevice, pCommandList);
		break;
	case FD_WRITE:
		send(server_sock, sendBuffer.data(), BUFSIZE, 0);
		break;
	case FD_CLOSE:
		cout << "서버가 종료되었습니다.\n";
		break;
	}

}


pair<int, int> GameFramework::GetClientSize() {
	return { clientWidth , clientHeight };
}



GameFramework::GameFramework() {
	instanceHandle = NULL;
	windowHandle = NULL;

	// MSAA 다중 샘플링
	msaa4xEnable = false;
	msaa4xLevel = 0;

	clientWidth = C_WIDTH;
	clientHeight = C_HEIGHT;

	dsvDescriptorIncrementSize = 0;
	fenceValues.fill(0);
	rtvDescriptorIncrementSize = 0;

	oldCursorPos = POINT();
	isClick = false;
	rtvCPUDescriptorHandles.fill(D3D12_CPU_DESCRIPTOR_HANDLE());
	// 현재 스왑체인의 후면 버퍼의 인덱스
	swapChainBufferCurrentIndex = 0;
	fenceEvent = NULL;
	//PushScene(make_shared<Scene>());

}

GameFramework::~GameFramework() {

}



void GameFramework::CreateDirect3DDevice() {
	HRESULT hResult;

	UINT nDxgiFactoryFlag = 0;

	// pDxgiFactory에 팩토리 생성
	hResult = CreateDXGIFactory2(nDxgiFactoryFlag, __uuidof(IDXGIFactory4), (void**)&pDxgiFactory);

	ComPtr<IDXGIAdapter1> pAdapter = NULL;

	for (int i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); ++i) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pAdapter->GetDesc1(&dxgiAdapterDesc);		// 어댑터의 정보를 가져온다.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { // 어댑터가 소프트웨어이면 건너뜀.
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice))) { // 적절한 어댑터 찾을 경우
			break;
		}
	}
	if (!pAdapter) {// 적절한 어댑터가 없을 경우 
		pDxgiFactory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&pAdapter);
		hResult = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice);
	}


	::cbvSrvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::rtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::dsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// 다중 샘플링 정보
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevel;
	msaaQualityLevel.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevel.SampleCount = 4;
	msaaQualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevel.NumQualityLevels = 0;
	hResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevel, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msaa4xLevel = msaaQualityLevel.NumQualityLevels;

	// 1 초과일시 msaa 가능
	msaa4xEnable = (msaa4xLevel > 1) ? true : false;

	// 펜스 
	hResult = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence);

	for (UINT64 fenceValue : fenceValues) {
		fenceValue = 0;
	}

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void GameFramework::CreateCommandQueueList() {
	HRESULT hResult;

	// 명령 대기열 생성
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = pDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&pCommandQueue);

	// 명령 할당자 생성
	hResult = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&pCommandAllocator);

	// 명령 리스트 생성
	hResult = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), NULL, __uuidof(ID3D12CommandList), (void**)&pCommandList);
	hResult = pCommandList->Close();

}

void GameFramework::CreateRtvAndDsvDescriptorHeaps() {
	// 렌더타겟 서술자 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	// 쉐도우맵 + G Buffer + 포스트 버퍼 + 후면버퍼
	descriptorHeapDesc.NumDescriptors = nSwapChainBuffer + NUM_G_BUFFER + NUM_SHADOW_MAP + 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pRtvDescriptorHeap);
	rtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 깊이 스텐실 서술자 힙 생성
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pDsvDescriptorHeap);
	dsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	GetClientRect(windowHandle, &rcClient);    // 클라이언트 크기를 rcClient에 저장

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = nSwapChainBuffer;    // 스왑체인(렌더 타켓) 버퍼의 개수
	dxgiSwapChainDesc.BufferDesc.Width = clientWidth;    // 윈도우 클라이언트 영역의 가로 크기 
	dxgiSwapChainDesc.BufferDesc.Height = clientHeight;    // 윈도우 클라이언트 영역의 세로 크기 
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // 픽셀을 RGBA에 8bit 형태로 그리기
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;    // RefreshRate : 화면 갱신률(프레임) //분자
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;    // 분모
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // 출력용 버퍼로 쓰겠다고 지정하는것.        읽기전용, 쉐이더의 입력으로 사용, 공유, 무순서화 접근 등의 형태가 있다.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // 전면 버퍼와 후면버퍼를 교체하고 버퍼 내용을 폐기(유지 하지 않으면 속도가 빠라짐)한다.
	dxgiSwapChainDesc.OutputWindow = windowHandle;    // 출력할 윈도우
	dxgiSwapChainDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xLevel - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;    // 창모드를 쓸껀지?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = pDxgiFactory->CreateSwapChain(pCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)pDxgiSwapChain.GetAddressOf());

	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = pDxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

}

void GameFramework::CreateRenderTargetViews() {

	HRESULT hResult;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();    // 서술자 힙을 통해 시작주소를 가져온다.
	for (int i = 0; i < nSwapChainBuffer; ++i) {
		hResult = pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pRenderTargetBuffers[i]);    // 렌더타겟 버퍼를 생성한다.
		pDevice->CreateRenderTargetView(pRenderTargetBuffers[i].Get(), NULL, rtvCPUDescriptorHandle);    // 렌더타겟 뷰를 서술자 힙에 생성(적재)
		rtvCPUDescriptorHandles[i] = rtvCPUDescriptorHandle;
		rtvCPUDescriptorHandle.ptr += rtvDescriptorIncrementSize;    // 다음번 주소로 이동
	}
}

void GameFramework::CreateDepthStencilView() {
	HRESULT hResult;
	// 깊이-스텐실 버퍼(리소스)를 만들기 위한 정보들 입력
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = clientWidth;
	resourceDesc.Height = clientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xLevel - 1) : 0;
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
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// 깊이-스텐실 버퍼(리소스) 생성
	hResult = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&pDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	::ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;


	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), &depthStencilViewDesc, dsvCPUDescriptorHandle);    // 깊이-스텐실 뷰를 서술자 힙에 생성(적제) (뷰==서술자?)



}

void GameFramework::CreateGraphicsRootSignature() {
	HRESULT hResult;

	D3D12_DESCRIPTOR_RANGE pDescriptorRanges[6];

	pDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[0].NumDescriptors = 1;
	pDescriptorRanges[0].BaseShaderRegister = 5;	// t5 = albedoMap
	pDescriptorRanges[0].RegisterSpace = 0;
	pDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[1].NumDescriptors = 1;
	pDescriptorRanges[1].BaseShaderRegister = 6;	// t6 = normalMap
	pDescriptorRanges[1].RegisterSpace = 0;
	pDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[2].NumDescriptors = NUM_G_BUFFER;
	pDescriptorRanges[2].BaseShaderRegister = 7;	// t7 ~ t12
	pDescriptorRanges[2].RegisterSpace = 0;
	pDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[3].NumDescriptors = NUM_SHADOW_MAP;
	pDescriptorRanges[3].BaseShaderRegister = 21;	// t21~
	pDescriptorRanges[3].RegisterSpace = 0;
	pDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[4].NumDescriptors = 1;
	pDescriptorRanges[4].BaseShaderRegister = 13;	// t13
	pDescriptorRanges[4].RegisterSpace = 0;
	pDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[5].NumDescriptors = 1;
	pDescriptorRanges[5].BaseShaderRegister = 14;	// t14
	pDescriptorRanges[5].RegisterSpace = 0;
	pDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	// 루트 시그니처는 이후 계속 수정 

	D3D12_ROOT_PARAMETER pRootParameters[17];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[0].Descriptor.ShaderRegister = 1; //Camera //shader.hlsl의 레지스터 번호 (예시 register(b1) )
	pRootParameters[0].Descriptor.RegisterSpace = 0;
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[1].Constants.Num32BitValues = 16;
	pRootParameters[1].Constants.ShaderRegister = 2; // 오브젝트 인스턴스의 의 월드행렬
	pRootParameters[1].Constants.RegisterSpace = 0;
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[2].Descriptor.ShaderRegister = 3; // 맵 내 빛의 정보 ( 사용할 빛에 대해서만 )
	pRootParameters[2].Descriptor.RegisterSpace = 0;
	pRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[3].Descriptor.ShaderRegister = 4; // Material
	pRootParameters[3].Descriptor.RegisterSpace = 0;
	pRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[4].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[0];
	pRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// Albedo Texture

	pRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[5].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[1];
	pRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// Normal Texture

	pRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[6].Descriptor.ShaderRegister = 7; // 스킨드 오브젝트 오브셋 행렬
	pRootParameters[6].Descriptor.RegisterSpace = 0;
	pRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[7].Descriptor.ShaderRegister = 8; // 스킨드 오브젝트 월드 변환 행렬
	pRootParameters[7].Descriptor.RegisterSpace = 0;
	pRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[8].Constants.Num32BitValues = 3;
	pRootParameters[8].Constants.ShaderRegister = 9; // 이펙트의 인덱스 정보
	pRootParameters[8].Constants.RegisterSpace = 0;
	pRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[9].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[2];
	pRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// G Buffer
	
	pRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[10].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[3];
	pRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// Shadow Map

	pRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[11].Constants.Num32BitValues = 1;
	pRootParameters[11].Constants.ShaderRegister = 5; // int값
	pRootParameters[11].Constants.RegisterSpace = 0;
	pRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[12].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[4];
	pRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// Emissive Texture

	pRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[13].Constants.Num32BitValues = 1;
	pRootParameters[13].Constants.ShaderRegister = 6; // float값
	pRootParameters[13].Constants.RegisterSpace = 0;
	pRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[14].Constants.Num32BitValues = 1;
	pRootParameters[14].Constants.ShaderRegister = 10; // shadow map을 위한 light index
	pRootParameters[14].Constants.RegisterSpace = 0;
	pRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[15].Descriptor.ShaderRegister = 11; // 월드 정보(시간)
	pRootParameters[15].Descriptor.RegisterSpace = 0;
	pRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[16].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[5];
	pRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// compute shader를 거친 후 출력 이미지


	D3D12_STATIC_SAMPLER_DESC samplerDesc[3];
	::ZeroMemory(samplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC) * 3);
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// 선형 필터링
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// 텍스처 타일링을 반복
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].MipLODBias = 0;
	samplerDesc[0].MaxAnisotropy = 1;
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[0].MinLOD = 0.0f;
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[0].ShaderRegister = 0;	// s0
	samplerDesc[0].RegisterSpace = 0;
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;	 // Clamp
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].MipLODBias = 0;
	samplerDesc[1].MaxAnisotropy = 1;
	samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[1].MinLOD = 0.0f;
	samplerDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[1].ShaderRegister = 1;	// s1
	samplerDesc[1].RegisterSpace = 0;
	samplerDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	 // 깊이값 비교를 위한 샘플러
	samplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[2].MipLODBias = 0;
	samplerDesc[2].MaxAnisotropy = 1;
	samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
	samplerDesc[2].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[2].MinLOD = 0.0f;
	samplerDesc[2].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[2].ShaderRegister = 2;	// s2
	samplerDesc[2].RegisterSpace = 0;
	samplerDesc[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootSignatureDesc.NumParameters = _countof(pRootParameters);
	rootSignatureDesc.pParameters = pRootParameters;
	rootSignatureDesc.NumStaticSamplers = _countof(samplerDesc);
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.Flags = rootSignatureFlags;

	ComPtr<ID3DBlob> pSignatureBlob = NULL;
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);

	hResult = pDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pRootSignature);
}

void GameFramework::CreateComputeRootSignature() {
	D3D12_DESCRIPTOR_RANGE pDescriptorRanges[3];

	pDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[0].NumDescriptors = 1;
	pDescriptorRanges[0].BaseShaderRegister = 15; //t15: 조명처리까지 끝난 텍스처
	pDescriptorRanges[0].RegisterSpace = 0;
	pDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[1].NumDescriptors = NUM_G_BUFFER;
	pDescriptorRanges[1].BaseShaderRegister = 16; //t16: 깊이 텍스처
	pDescriptorRanges[1].RegisterSpace = 0;
	pDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pDescriptorRanges[2].NumDescriptors = 1;
	pDescriptorRanges[2].BaseShaderRegister = 0; //u0: 실제 결과물
	pDescriptorRanges[2].RegisterSpace = 0;
	pDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pRootParameters[3];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[0].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[0];
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// 조명 처리까지 끝난 텍스처

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[1].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[1];
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// 깊이 텍스처

	pRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[2].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[2]; //RWTexture2D
	pRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootSignatureDesc.NumParameters = _countof(pRootParameters);
	rootSignatureDesc.pParameters = pRootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = NULL;
	rootSignatureDesc.Flags = rootSignatureFlags;

	ComPtr<ID3DBlob> pSignatureBlob = NULL;
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	HRESULT hResult;
	hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);

	hResult = pDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pComputeRootSignature);
}

// 쉐이더 관련 변수
bool GameFramework::InitShader()
{
	return shaderManager.InitShader(pDevice, pRootSignature, pComputeRootSignature);
}

void GameFramework::NoticeCloseToServer() {
	if (pScenes.top()) pScenes.top()->NoticeCloseToServer();
}

// get, set 함수

TextureManager& GameFramework::GetTextureManager() {
	return textureManager;
}

GameObjectManager& GameFramework::GetGameObjectManager() {
	return gameObjectManager;
}


SoundManager& GameFramework::GetSoundManager() {
	return pSoundManager;
}

shared_ptr<Shader> GameFramework::GetShader(const string& _name) {
	return shaderManager.GetShader(_name);
}

void GameFramework::InitOldCursor() {
	POINT mid{ C_WIDTH / 2, C_HEIGHT / 2 };
	ClientToScreen(hWnd, &mid);
	oldCursorPos = mid;
	SetCursorPos(mid.x, mid.y);
}

void GameFramework::InitBuffer() {

	// 기존 렌더타겟의 다음 위치를 구한다.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCPUDescriptorHandle.ptr += (rtvDescriptorIncrementSize * nSwapChainBuffer);
	
	//  텍스처를 만든다.
	pGBuffer = make_shared<Texture>(NUM_G_BUFFER, RESOURCE_TEXTURE2D, 0, 1);
	pShadowMap = make_shared<Texture>(NUM_SHADOW_MAP, RESOURCE_TEXTURE2D, 0, 1);
	pPostBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pComputeBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	//pDestBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);

	DXGI_FORMAT format[NUM_G_BUFFER] = {
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32_FLOAT }
	;
	D3D12_CLEAR_VALUE clearValue[NUM_G_BUFFER] = {
		{DXGI_FORMAT_R32G32B32A32_FLOAT, {1.0f, 0.0f, 0.0f, 1.0f}},
		{DXGI_FORMAT_R32G32B32A32_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f}},
		{DXGI_FORMAT_R32G32B32A32_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f}},
		{DXGI_FORMAT_R32G32B32A32_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f}},
		{DXGI_FORMAT_R32G32B32A32_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f}},
		{DXGI_FORMAT_R32_FLOAT, {1.0f}},
	};

	D3D12_CLEAR_VALUE shadowMapClearValue = { DXGI_FORMAT_R32_FLOAT, {1.0f} };
	// 텍스처 생성. 렌더타겟과 같은 형태로 만들어둔다.
	for (int i = 0; i < NUM_G_BUFFER; ++i) {
		pGBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, format[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue[i], RESOURCE_TEXTURE2D, i);
	}
	for (int i = 0; i < NUM_SHADOW_MAP; ++i) {
		pShadowMap->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &shadowMapClearValue, RESOURCE_TEXTURE2D, i);
	}

	pPostBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue[0], RESOURCE_TEXTURE2D, 0);
	// uav도 사용할것이므로 플래그를 추가해준다.
	pComputeBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, RESOURCE_TEXTURE2D, 0);
	//pDestBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, NULL, RESOURCE_TEXTURE2D, 0);
	
	// g buffer는 루트시그니처 9번을 사용.
	Shader::CreateShaderResourceViews(pDevice, pGBuffer, 0, 9);
	// g buffer에 대해 srv를 만들어둔다. 
	Shader::CreateComputeShaderResourceViews(pDevice, pGBuffer, 0, 1);

	// 쉐도우맵은 10번을 사용
	Shader::CreateShaderResourceViews(pDevice, pShadowMap, 0, 10);

	// 포스트 버퍼에 대한 srv를 만든다. 
	Shader::CreateShaderResourceViews(pDevice, pPostBuffer, 0, 16);
	Shader::CreateComputeShaderResourceViews(pDevice, pPostBuffer, 0, 0);

	//// dest buffer에는 그래픽, 컴퓨트 힙에 하나씩 srv를 만든다.
	//Shader::CreateShaderResourceViews(pDevice, pDestBuffer, 0, 16);

	// 컴퓨트 버퍼는 컴퓨트 쉐이더에서 쓸 uav를 하나 만들어준다.
	Shader::CreateShaderResourceViews(pDevice, pComputeBuffer, 0, 16);
	Shader::CreateComputeUnorderedAccessView(pDevice, pComputeBuffer, 0);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	// 렌더타겟뷰 주소를 담을 공간 할당
	GBufferCPUDescriptorHandles.resize(NUM_G_BUFFER);
	shadowMapCPUDescriptorHandles.resize(NUM_SHADOW_MAP);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvPtr = rtvCPUDescriptorHandle;

	for (UINT i = 0; i < NUM_G_BUFFER; i++)
	{
		d3dRenderTargetViewDesc.Format = format[i];
		// i번째 리소스에 대한 렌더타겟뷰를 생성후 주소를 저장
		ID3D12Resource* pd3dTextureResource = pGBuffer->GetResource(i).Get();

		// 기본 렌더타겟뷰 다음 주소에 바로 만든다.
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvPtr);
		GBufferCPUDescriptorHandles[i] = rtvPtr;

		rtvPtr.ptr += ::rtvDescriptorIncrementSize;
	}

	for (UINT i = 0; i < NUM_SHADOW_MAP; i++)
	{
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		// i번째 리소스에 대한 렌더타겟뷰를 생성후 주소를 저장
		ID3D12Resource* pd3dTextureResource = pShadowMap->GetResource(i).Get();

		// g 버퍼 주소에 이어서 만든다.
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvPtr);
		shadowMapCPUDescriptorHandles[i] = rtvPtr;

		rtvPtr.ptr += ::rtvDescriptorIncrementSize;
	}

	{
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ID3D12Resource* pd3dTextureResource = pPostBuffer->GetResource(0).Get();
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvPtr);
		postBufferCPUDescriptorHandle = rtvPtr;

		rtvPtr.ptr += ::rtvDescriptorIncrementSize;
	}
}

const shared_ptr<Scene>& GameFramework::GetCurrentScene() const {
	//if (!pScenes.empty()) {
	return pScenes.top();
	//}
}

void GameFramework::FrameAdvance() {

	gameTimer.Tick(0.0f);

	// 명령 할당자와 명령 리스트를 리셋한다.
	HRESULT hResult = pCommandAllocator->Reset();
	hResult = pCommandList->Reset(pCommandAllocator.Get(), NULL);
	vector<ComPtr<ID3D12CommandList>>  pCommandLists;
	float timeElapsed = gameTimer.GetTimeElapsed();
	// 입력을 받을 때 플레이어의 움직임을 저장한다.
	ProcessInput();

	if (!pScenes.empty()) {	// 씬 진행(애니메이트). 스택의 맨 위 원소에 대해 진행
		// 저장된 이동, 회전으로 먼저 충돌체크를 진행해본 후 실제로 플레이어를 움직인다.
		pScenes.top()->AnimateObjects(pScenes.top()->CheckCollision(timeElapsed), timeElapsed, pDevice, pCommandList);
	}

	// 사라져야 할 오브젝트들을 쉐이더의 오브젝트 벡터에서 없앤다.
	Shader::UpdateShadersObject();

	// 현재 렌더 타겟에 대한 Present가 끝나기를 기다림.  (PRESENT = 프리젠트 상태, RENDER_TARGET = 렌더 타겟 상태
	D3D12_RESOURCE_BARRIER resourceBarrier;
	ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = pRenderTargetBuffers[swapChainBufferCurrentIndex].Get();
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &resourceBarrier);
	// 루트 시그니처를 Set
	pCommandList->SetGraphicsRootSignature(pRootSignature.Get());
	pCommandList->SetComputeRootSignature(pComputeRootSignature.Get());
	
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// 후면 버퍼의 핸들
	D3D12_CPU_DESCRIPTOR_HANDLE swapChainDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	swapChainDescriptorHandle.ptr += (rtvDescriptorIncrementSize * swapChainBufferCurrentIndex);

	// 깊이 스텐실 버퍼의 핸들
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 그림자 맵 렌더링

	Shader::SetDescriptorHeap(pCommandList);
	
	if (!pScenes.empty()) {
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvShadowCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[1];

		for (int i = 0; i < NUM_SHADOW_MAP; ++i)
		{
			rtvShadowCPUDescriptorHandles[0].ptr = shadowMapCPUDescriptorHandles[i].ptr;

			// 이전 그림자 맵을 그릴 때 남은 깊이값을 지운다.
			pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			SynchronizeResourceTransition(pCommandList.Get(), pShadowMap->GetResource(i).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

			// i번째 쉐도우맵을 렌더타겟으로 지정한다.
			pCommandList->OMSetRenderTargets(1, rtvShadowCPUDescriptorHandles, TRUE, &dsvCPUDescriptorHandle);
			pCommandList->ClearRenderTargetView(rtvShadowCPUDescriptorHandles[0], pClearColor, 0, NULL);

			// i번째 조명에 대한 쉐도우맵 렌더링을 한다.
			pScenes.top()->RenderShadowMap(pCommandList, i);

			SynchronizeResourceTransition(pCommandList.Get(), pShadowMap->GetResource(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		}
		delete[] rtvShadowCPUDescriptorHandles;
	}

	pShadowMap->UpdateShaderVariable(pCommandList);
	
	

	// G Buffer 렌더링
	// 미리 그릴 버퍼의 핸들
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[NUM_G_BUFFER];

	for (int i = 0; i < NUM_G_BUFFER; ++i)
		SynchronizeResourceTransition(pCommandList.Get(), pGBuffer->GetResource(i).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	for (int i = 0; i < NUM_G_BUFFER; ++i) {
		rtvCPUDescriptorHandles[i].ptr = GBufferCPUDescriptorHandles[i].ptr;
		pCommandList->ClearRenderTargetView(GBufferCPUDescriptorHandles[i], pClearColor, 0, NULL);
	}

	// G Buffer 및 조명 처리전의 씬을 그리기 위한 버퍼들을 렌더타겟에 Set한다. 
	pCommandList->OMSetRenderTargets(NUM_G_BUFFER, rtvCPUDescriptorHandles, FALSE, &dsvCPUDescriptorHandle);
	pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// 해당 씬에 대한 G Buffer 및 조명을 제외한 씬을 그린다.
	if (!pScenes.empty()) {
		pScenes.top()->PreRender(pCommandList, timeElapsed);
	}

	for (int i = 0; i < NUM_G_BUFFER; ++i)
		SynchronizeResourceTransition(pCommandList.Get(), pGBuffer->GetResource(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

	delete[] rtvCPUDescriptorHandles;

	// lobbyScene일 경우 후면버퍼를, playScene은 postBuffer를 렌더타겟으로 설정한다.

	
	// 씬 렌더링
	if (!pScenes.empty()) {
		if (dynamic_pointer_cast<LobbyScene>(pScenes.top())) {
			pCommandList->OMSetRenderTargets(1, &swapChainDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
			pCommandList->ClearRenderTargetView(swapChainDescriptorHandle, pClearColor, 0, NULL);
			pScenes.top()->Render(pCommandList, timeElapsed);
		}
		else {
			// 포스트 버퍼에 조명처리한 현재 씬을 그린다.
			pCommandList->OMSetRenderTargets(1, &postBufferCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
			pCommandList->ClearRenderTargetView(postBufferCPUDescriptorHandle, pClearColor, 0, NULL);
			pScenes.top()->LightingRender(pCommandList, timeElapsed);
			pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			// 포스트 버퍼에 대한 블러 처리를 진행한다.
			Shader::SetComputeDescriptorHeap(pCommandList);
			static_pointer_cast<BlurComputeShader>(GetShader("BlurComputeShader"))->Dispatch(pCommandList);

			Shader::SetDescriptorHeap(pCommandList);
			pCommandList->OMSetRenderTargets(1, &swapChainDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
			pCommandList->ClearRenderTargetView(swapChainDescriptorHandle, pClearColor, 0, NULL);
			pScenes.top()->Render(pCommandList, timeElapsed);

		}
		
	}




	// 현재 렌더 타겟에 대한 렌더링이 끝나기를 기다린다.
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pCommandList->ResourceBarrier(1, &resourceBarrier);
	 
	//명령 리스트를 닫힌 상태로 만든다. 
	hResult = pCommandList->Close();
	//명령 리스트를 명령 큐에 추가하여 실행한다. 
	pCommandLists = { pCommandList.Get() };

	pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());

	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다. 
	WaitForGpuComplete();

	if (!pScenes.empty()) {
		pScenes.top()->PostRender();
	}

	//	스왑체인을 프리젠트한다.
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// 다음 프레임으로 이동, (다음 버퍼로 이동)
	MoveToNextFrame();

	// FPS 표시
	wstring titleString = L"FPS : " + to_wstring(gameTimer.GetFPS());
	SetWindowText(windowHandle, (LPCWSTR)titleString.c_str());
}

void GameFramework::WaitForGpuComplete() {
	UINT64 fenceValue = ++fenceValues[swapChainBufferCurrentIndex]; // 현재 버퍼의 펜스값을 증가
	HRESULT hResult = pCommandQueue->Signal(pFence.Get(), fenceValue);
	if (pFence->GetCompletedValue() < fenceValue) {
		hResult = pFence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);					// GPU가 처리를 끝낼때 까지 기다림
	}
}

void GameFramework::MoveToNextFrame() {
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex(); // 다음 후면 버퍼로 변경
	WaitForGpuComplete();
}

void GameFramework::ChangeSwapChainState() {

	WaitForGpuComplete();
	BOOL fullScreenState;

	// 현재 모드 반전
	pDxgiSwapChain->GetFullscreenState(&fullScreenState, NULL);
	pDxgiSwapChain->SetFullscreenState(~fullScreenState, NULL);


	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = clientWidth;
	dxgiTargetParameters.Height = clientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	pDxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	for (int i = 0; i < nSwapChainBuffer; i++) {
		if (pRenderTargetBuffers[i]) {
			pRenderTargetBuffers[i].Reset();
		}
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	pDxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	pDxgiSwapChain->ResizeBuffers(nSwapChainBuffer, clientWidth,
		clientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews();
}


void GameFramework::ProcessInput() {

	static array<UCHAR, 256> keysBuffers;
	static array<bool, 256> keysDownStateBuffers;	// 키를 누른상태인지 확인한는 변수, 
	static array<bool, 256> keysDownBuffers;		// 현재 키가 눌린 순간인지 저장하는 변수
	static array<bool, 256> keysUpBuffers;		// 현재 키가 안눌린 순간인지 저장하는 변수
	bool processedByScene = false;

	keysDownBuffers.fill(false);
	keysUpBuffers.fill(false);
	if (GetKeyboardState((PBYTE)keysBuffers.data())) {	// 키보드로 부터 입력데이터를 받으면
		for (int i = 0; i < 256; ++i) {
			if ((keysBuffers[i] & 0xF0) && !keysDownStateBuffers[i]) {	// 키를 누르는 순간
				keysDownStateBuffers[i] = true;
				keysDownBuffers[i] = true;
			}
			else if (!(keysBuffers[i] & 0xF0) && keysDownStateBuffers[i]) {	// 키를 떼는 순간.
				keysDownStateBuffers[i] = false;
				keysUpBuffers[i] = true;
			}
		}
		
		if (keysDownBuffers['F']) {
			ChangeSwapChainState();
		}
		// 일시정지
		if (keysDownBuffers['P']) {
			//PushScene(make_shared<Scene>("pause"));
		}
		// 재시작
		if (keysDownBuffers['R']) {
		}
		
		if (GetCapture() == windowHandle)
		{
			POINT curCursorPos;
			XMFLOAT2 delta;

			GetCursorPos(&curCursorPos);
			delta.x = (float)(curCursorPos.x - oldCursorPos.x);
			delta.y = (float)(curCursorPos.y - oldCursorPos.y);
			//SetCursorPos(oldCursorPos.x, oldCursorPos.y);

			SetCursor(NULL);
			InitOldCursor();

			if (!pScenes.empty()) pScenes.top()->ProcessCursorMove(delta);
		}

		// 씬의 키보드입력 처리
		if (!pScenes.empty()) {
			pScenes.top()->ProcessKeyboardInput(keysDownBuffers, keysBuffers, gameTimer.GetTimeElapsed(), pDevice, pCommandList);
		}
	}
}


void GameFramework::PushScene(const shared_ptr<Scene>& _pScene) {
	pScenes.push(_pScene);
}


void GameFramework::LoadingScene(const shared_ptr<Scene>& _pScene) {
	GameFramework& gameFramework = *spInstance;
	gameFramework.pCommandList->Reset(gameFramework.pCommandAllocator.Get(), NULL);

	_pScene->Init(pDevice, pCommandList);

	gameFramework.pCommandList->Close();

	vector<ComPtr<ID3D12CommandList>> pCommandLists = { gameFramework.pCommandList.Get() };
	gameFramework.pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());
	gameFramework.WaitForGpuComplete();
}

void GameFramework::PopScene() {
	if (!pScenes.empty()) {
		pScenes.pop();
	}
}
 
void GameFramework::ChangeScene(const shared_ptr<Scene>& _pScene) {
	PopScene();
	PushScene(_pScene);
}

void GameFramework::ClearScene() {

	while (!pScenes.empty()) {
		pScenes.pop();
	}
}

shared_ptr<Texture> GameFramework::GetGBuffer() const {
	return pGBuffer;
}

shared_ptr<Texture> GameFramework::GetShadowMap() const {
	return pShadowMap;
}

shared_ptr<Texture> GameFramework::GetPostBuffer() const {
	return pPostBuffer;
}

shared_ptr<Texture> GameFramework::GetComputeBuffer() const {
	return pComputeBuffer;
}

shared_ptr<Texture> GameFramework::GetDestBuffer() const {
	return pDestBuffer;
}

void GameFramework::CreateShaderVariables() {
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256의 배수

	pcbFrameworkInfo = ::CreateBufferResource(pDevice.Get(), pCommandList.Get(), NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pcbFrameworkInfo->Map(0, NULL, (void**)&pcbMappedFrameworkInfo);
}
void GameFramework::UpdateShaderVariables() {
	float tempTimeElapsed = gameTimer.GetTimeElapsed();
	pcbMappedFrameworkInfo->currentTime = (float)gameTimer.GetTimeElapsed();
	pcbMappedFrameworkInfo->elapsedTime = (float)gameTimer.GetTimeElapsed();

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pcbFrameworkInfo->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(15, d3dGpuVirtualAddress);
}

