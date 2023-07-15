#include "stdafx.h"
#include "GameFramework.h"

unique_ptr<GameFramework> GameFramework::spInstance;


void GameFramework::Create(HINSTANCE _hInstance, HWND _hMainWnd) {

	if (!spInstance) {		// ������ ��ũ �ν��Ͻ��� ���������� ���� ��� 
		spInstance.reset(new GameFramework());
		GameFramework& gameFramework = *spInstance;

		gameFramework.instanceHandle = _hInstance;
		gameFramework.windowHandle = _hMainWnd;



		gameFramework.CreateDirect3DDevice();    // ������� ����̽��� �����ؾ� ��� ��⿭�̳� ������ �� ���� ������ �� �ִ�.
		gameFramework.CreateCommandQueueList();
		gameFramework.CreateRtvAndDsvDescriptorHeaps();
		gameFramework.CreateSwapChain();    // DxgiFactory, CommandQueue, RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
		gameFramework.CreateRenderTargetViews();
		gameFramework.CreateDepthStencilView();
		gameFramework.CreateGraphicsRootSignature();
		gameFramework.CreateComputeRootSignature();

		CoInitialize(NULL);
		gameFramework.pCommandList->Reset(gameFramework.pCommandAllocator.Get(), NULL);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = gameFramework.pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (gameFramework.rtvDescriptorIncrementSize * gameFramework.nSwapChainBuffer);

		gameFramework.pSoundManager.Init(_hMainWnd);

		// ���̴� ����
		if (!gameFramework.InitShader()) {
			cout << "���̴� ���� ����\n";
		}
		// ���̴��� ���� ����(����ð� ��)�� �Ҵ�.
		gameFramework.CreateShaderVariables();

		// cbv, srv�� ��� ���� ���� ��ũ���� �� ����
		Shader::CreateCbvSrvUavDescriptorHeaps(gameFramework.pDevice, 0, 300, 1);
		Shader::CreateComputeDescriptorHeaps(gameFramework.pDevice, 0, 200, MAX_LIGHTS + NUM_SHADOW_MAP);



		// �ؽ�ó ����� ���� TextLayer �ν��Ͻ� �ʱ�ȭ
		TextLayer::Create(nSwapChainBuffer, gameFramework.pDevice, gameFramework.pCommandQueue, gameFramework.pRenderTargetBuffers, gameFramework.clientWidth, gameFramework.clientHeight);
		
		// ����׿� ��Ʈ�ڽ� �޽� ����
		gameFramework.hitBoxMesh.Create(gameFramework.pDevice, gameFramework.pCommandList);
		
		// G Buffer ����
		gameFramework.InitBuffer();

		// ��ƼŬ�� �׸��� ���� ���ҽ��� ����
		Shader::InitParticleResource(gameFramework.pDevice, gameFramework.pCommandList);

		// ���ʾ� ����
		shared_ptr<Scene> pScene = make_shared<LobbyScene>();
		gameFramework.LoadingScene(pScene);
		gameFramework.PushScene(pScene);




		// ��Ʈ�ڽ��� �޽� ����
		//gameFramework.meshManager.GetHitBoxMesh().Create(gameFramework.pDevice, gameFramework.pCommandList);

		gameFramework.pCommandList->Close();

		vector<ComPtr<ID3D12CommandList>> pCommandLists = { gameFramework.pCommandList.Get() };
		gameFramework.pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());


		gameFramework.WaitForGpuComplete();
		//startScene->ReleaseUploadBuffers();
		gameFramework.gameTimer.Reset();    // Ÿ�̸� ����
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

		if (isClick) return; // ó�� �ѹ� ���콺�� Ŭ�� �������� ����
		break;
	case WM_LBUTTONUP:
		if (!isClick) return;
		break;
	}

	isClick = !isClick;	// Ŭ�� ���¸� �ٲپ��ش�.
	if (pScenes.top()) pScenes.top()->ProcessMouseInput(_type, _pos);
}

void GameFramework::ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	/// wParam : ����, lParam : select�� �ʵ�

	if (WSAGETSELECTERROR(_lParam)) {
		SockErrorDisplay(WSAGETSELECTERROR(_lParam));
		return;
	}
	// �޽��� ó��
	switch (WSAGETSELECTEVENT(_lParam)) {
	case FD_READ:
		if (pScenes.top()) pScenes.top()->ProcessSocketMessage(pDevice, pCommandList);
		break;
	case FD_WRITE:
		send(server_sock, sendBuffer.data(), BUFSIZE, 0);
		break;
	case FD_CLOSE:
		cout << "������ ����Ǿ����ϴ�.\n";
		break;
	}

}


pair<int, int> GameFramework::GetClientSize() {
	return { clientWidth , clientHeight };
}



GameFramework::GameFramework() {
	instanceHandle = NULL;
	windowHandle = NULL;

	// MSAA ���� ���ø�
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
	postBufferCPUDescriptorHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
	// ���� ����ü���� �ĸ� ������ �ε���
	swapChainBufferCurrentIndex = 0;
	fenceEvent = NULL;
	//PushScene(make_shared<Scene>());

}

GameFramework::~GameFramework() {

}



void GameFramework::CreateDirect3DDevice() {
	HRESULT hResult;

	UINT nDxgiFactoryFlag = 0;

	// pDxgiFactory�� ���丮 ����
	hResult = CreateDXGIFactory2(nDxgiFactoryFlag, __uuidof(IDXGIFactory4), (void**)&pDxgiFactory);

	ComPtr<IDXGIAdapter1> pAdapter = NULL;

	for (int i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); ++i) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pAdapter->GetDesc1(&dxgiAdapterDesc);		// ������� ������ �����´�.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { // ����Ͱ� ����Ʈ�����̸� �ǳʶ�.
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice))) { // ������ ����� ã�� ���
			break;
		}
	}
	if (!pAdapter) {// ������ ����Ͱ� ���� ��� 
		pDxgiFactory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&pAdapter);
		hResult = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice);
	}


	::cbvSrvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::rtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::dsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// ���� ���ø� ����
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevel;
	msaaQualityLevel.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevel.SampleCount = 4;
	msaaQualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevel.NumQualityLevels = 0;
	hResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevel, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msaa4xLevel = msaaQualityLevel.NumQualityLevels;

	// 1 �ʰ��Ͻ� msaa ����
	msaa4xEnable = (msaa4xLevel > 1) ? true : false;

	// �潺 
	hResult = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence);

	for (UINT64 fenceValue : fenceValues) {
		fenceValue = 0;
	}

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void GameFramework::CreateCommandQueueList() {
	HRESULT hResult;

	// ��� ��⿭ ����
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = pDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&pCommandQueue);

	// ��� �Ҵ��� ����
	hResult = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&pCommandAllocator);

	// ��� ����Ʈ ����
	hResult = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), NULL, __uuidof(ID3D12CommandList), (void**)&pCommandList);
	hResult = pCommandList->Close();

}

void GameFramework::CreateRtvAndDsvDescriptorHeaps() {
	// ����Ÿ�� ������ �� ����
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	// �ĸ���� + ������� + G Buffer + ����Ʈ ���� + ���̴� �ؽ�ó + baked ������ ��
	descriptorHeapDesc.NumDescriptors = nSwapChainBuffer + NUM_G_BUFFER + NUM_SHADOW_MAP + 1 + 1 + MAX_LIGHTS;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pRtvDescriptorHeap);
	rtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ���� ���ٽ� ������ �� ����
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pDsvDescriptorHeap);
	dsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	GetClientRect(windowHandle, &rcClient);    // Ŭ���̾�Ʈ ũ�⸦ rcClient�� ����

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = nSwapChainBuffer;    // ����ü��(���� Ÿ��) ������ ����
	dxgiSwapChainDesc.BufferDesc.Width = clientWidth;    // ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Height = clientHeight;    // ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // �ȼ��� RGBA�� 8bit ���·� �׸���
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;    // RefreshRate : ȭ�� ���ŷ�(������) //����
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;    // �и�
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // ��¿� ���۷� ���ڴٰ� �����ϴ°�.        �б�����, ���̴��� �Է����� ���, ����, ������ȭ ���� ���� ���°� �ִ�.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // ���� ���ۿ� �ĸ���۸� ��ü�ϰ� ���� ������ ���(���� ���� ������ �ӵ��� ������)�Ѵ�.
	dxgiSwapChainDesc.OutputWindow = windowHandle;    // ����� ������
	dxgiSwapChainDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xLevel - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;    // â��带 ������?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = pDxgiFactory->CreateSwapChain(pCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)pDxgiSwapChain.GetAddressOf());
	
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = pDxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

}

void GameFramework::CreateRenderTargetViews() {

	HRESULT hResult;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();    // ������ ���� ���� �����ּҸ� �����´�.
	for (int i = 0; i < nSwapChainBuffer; ++i) {
		hResult = pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pRenderTargetBuffers[i]);    // ����Ÿ�� ���۸� �����Ѵ�.
		pDevice->CreateRenderTargetView(pRenderTargetBuffers[i].Get(), NULL, rtvCPUDescriptorHandle);    // ����Ÿ�� �並 ������ ���� ����(����)
		rtvCPUDescriptorHandles[i] = rtvCPUDescriptorHandle;
		rtvCPUDescriptorHandle.ptr += rtvDescriptorIncrementSize;    // ������ �ּҷ� �̵�
	}
}

void GameFramework::CreateDepthStencilView() {
	HRESULT hResult;
	// ����-���ٽ� ����(���ҽ�)�� ����� ���� ������ �Է�
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

	// ����-���ٽ� ����(���ҽ�) ����
	hResult = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&pDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	::ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;


	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), &depthStencilViewDesc, dsvCPUDescriptorHandle);    // ����-���ٽ� �並 ������ ���� ����(����) (��==������?)



}

void GameFramework::CreateGraphicsRootSignature() {
	HRESULT hResult;

	D3D12_DESCRIPTOR_RANGE pDescriptorRanges[7];

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

	pDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[6].NumDescriptors = 1;
	pDescriptorRanges[6].BaseShaderRegister = 15;	// t15
	pDescriptorRanges[6].RegisterSpace = 0;
	pDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ��Ʈ �ñ״�ó�� ���� ��� ���� 

	D3D12_ROOT_PARAMETER pRootParameters[18];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[0].Descriptor.ShaderRegister = 1; //Camera //shader.hlsl�� �������� ��ȣ (���� register(b1) )
	pRootParameters[0].Descriptor.RegisterSpace = 0;
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[1].Constants.Num32BitValues = 16;
	pRootParameters[1].Constants.ShaderRegister = 2; // ������Ʈ �ν��Ͻ��� �� �������
	pRootParameters[1].Constants.RegisterSpace = 0;
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[2].Descriptor.ShaderRegister = 3; // �� �� ���� ���� ( ����� ���� ���ؼ��� )
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
	pRootParameters[6].Descriptor.ShaderRegister = 7; // ��Ų�� ������Ʈ ����� ���
	pRootParameters[6].Descriptor.RegisterSpace = 0;
	pRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[7].Descriptor.ShaderRegister = 8; // ��Ų�� ������Ʈ ���� ��ȯ ���
	pRootParameters[7].Descriptor.RegisterSpace = 0;
	pRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[8].Constants.Num32BitValues = 3;
	pRootParameters[8].Constants.ShaderRegister = 9; // ����Ʈ�� �ε��� ����
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
	pRootParameters[11].Constants.ShaderRegister = 5; // int��
	pRootParameters[11].Constants.RegisterSpace = 0;
	pRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[12].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[4];
	pRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// Emissive Texture

	pRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[13].Constants.Num32BitValues = 1;
	pRootParameters[13].Constants.ShaderRegister = 6; // float��
	pRootParameters[13].Constants.RegisterSpace = 0;
	pRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[14].Constants.Num32BitValues = 1;
	pRootParameters[14].Constants.ShaderRegister = 10; // shadow map�� ���� light index
	pRootParameters[14].Constants.RegisterSpace = 0;
	pRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[15].Descriptor.ShaderRegister = 11; // ���� ����(�ð�)
	pRootParameters[15].Descriptor.RegisterSpace = 0;
	pRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[16].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[5];
	pRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// compute shader�� ��ģ �� ��� �̹���

	pRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[17].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[6];
	pRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// G Buffer

	D3D12_STATIC_SAMPLER_DESC samplerDesc[3];
	::ZeroMemory(samplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC) * 3);
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// ���� ���͸�
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// �ؽ�ó Ÿ�ϸ��� �ݺ�
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
	samplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	 // ���̰� �񱳸� ���� ���÷�
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
	D3D12_DESCRIPTOR_RANGE pDescriptorRanges[6];

	pDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[0].NumDescriptors = 1;
	pDescriptorRanges[0].BaseShaderRegister = 15; //t15: ����ó������ ���� �ؽ�ó
	pDescriptorRanges[0].RegisterSpace = 0;
	pDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[1].NumDescriptors = NUM_G_BUFFER;
	pDescriptorRanges[1].BaseShaderRegister = 16; //t16 ~ t21: g buffer
	pDescriptorRanges[1].RegisterSpace = 0;
	pDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pDescriptorRanges[2].NumDescriptors = 1;
	pDescriptorRanges[2].BaseShaderRegister = 0; //u0: ���� �����
	pDescriptorRanges[2].RegisterSpace = 0;
	pDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[3].NumDescriptors = 1;
	pDescriptorRanges[3].BaseShaderRegister = 22; //t22 = ���̴� ���̾������� �ؽ�ó
	pDescriptorRanges[3].RegisterSpace = 0;
	pDescriptorRanges[3].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[4].NumDescriptors = 1;
	pDescriptorRanges[4].BaseShaderRegister = 23; //t23: baked shadow
	pDescriptorRanges[4].RegisterSpace = 0;
	pDescriptorRanges[4].OffsetInDescriptorsFromTableStart = 0;

	pDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[5].NumDescriptors = 1;
	pDescriptorRanges[5].BaseShaderRegister = 24; //t24: dynamic shadow
	pDescriptorRanges[5].RegisterSpace = 0;
	pDescriptorRanges[5].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pRootParameters[7];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[0].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[0];
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// ���� ó������ ���� �ؽ�ó

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[1].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[1];
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// g buffer

	pRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[2].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[2]; //RWTexture2D
	pRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[3].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[3];
	pRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// ���̴� �ؽ���

	pRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[4].Constants.Num32BitValues = 2;
	pRootParameters[4].Constants.ShaderRegister = 0; // b0 = ���̴��� ���� ���̰�, �׷��� ����
	pRootParameters[4].Constants.RegisterSpace = 0;
	pRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[5].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[4];
	pRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// baked shadow

	pRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[6].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[5];
	pRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// dynamic shadow


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

// ���̴� ���� ����
bool GameFramework::InitShader()
{
	return shaderManager.InitShader(pDevice, pRootSignature, pComputeRootSignature);
}

void GameFramework::NoticeCloseToServer() {
	if (pScenes.top()) pScenes.top()->NoticeCloseToServer();
}

// get, set �Լ�

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

	// ���� ����Ÿ���� ���� ��ġ�� ���Ѵ�.
	rtvDescriptorHeapCurrentHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvDescriptorHeapCurrentHandle.ptr += (rtvDescriptorIncrementSize * nSwapChainBuffer);

	//  �ؽ�ó�� �����.
	pGBuffer = make_shared<Texture>(NUM_G_BUFFER, RESOURCE_TEXTURE2D, 0, 1);
	pShadowMap = make_shared<Texture>(NUM_SHADOW_MAP, RESOURCE_TEXTURE2D, 0, 1);
	pDynamicShadowMap = make_shared<Texture>(NUM_SHADOW_MAP, RESOURCE_TEXTURE2D, 0, 1);
	
	pPostBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pComputeBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pWireFrameMap = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	//pDestBuffer = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);

	pBakedShadowMaps.resize(80);
	bakedShadowMapCPUDescriptorHandles.resize(80);
	for (auto& pBakedShadowMap : pBakedShadowMaps) {
		pBakedShadowMap = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	}

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
	// �ؽ�ó ����. ����Ÿ�ٰ� ���� ���·� �����д�.
	for (int i = 0; i < NUM_G_BUFFER; ++i) {
		pGBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, format[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue[i], RESOURCE_TEXTURE2D, i);
	}
	for (int i = 0; i < NUM_SHADOW_MAP; ++i) {
		// ��ǻƮ���̴����� ����ϹǷ� UNORDERED_ACCESS ���� �÷��׸� ���
		pShadowMap->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, RESOURCE_TEXTURE2D, i);
	}
	for (int i = 0; i < NUM_SHADOW_MAP; ++i) {
		pDynamicShadowMap->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &shadowMapClearValue, RESOURCE_TEXTURE2D, i);
	}
	for (auto& pBakedShadowMap : pBakedShadowMaps) {
		pBakedShadowMap->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &shadowMapClearValue, RESOURCE_TEXTURE2D, 0);
	}

	pPostBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue[0], RESOURCE_TEXTURE2D, 0);
	// uav�� ����Ұ��̹Ƿ� �÷��׸� �߰����ش�.
	pComputeBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, RESOURCE_TEXTURE2D, 0);
	pWireFrameMap->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue[0], RESOURCE_TEXTURE2D, 0);
	//pDestBuffer->CreateTexture(pDevice, C_WIDTH, C_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, NULL, RESOURCE_TEXTURE2D, 0);

	// g buffer�� ��Ʈ�ñ״�ó 9���� ���.
	Shader::CreateShaderResourceViews(pDevice, pGBuffer, 0, 9);
	// g buffer�� ���� srv�� �����д�. 
	Shader::CreateComputeShaderResourceViews(pDevice, pGBuffer, 0, 1);

	// �׷��Ƚ� �ܰ迡�� ����� ���� 10��, ��ǻƮ ���̴����� ����� �����ϱ� ���� uav�� ����
	Shader::CreateShaderResourceViews(pDevice, pShadowMap, 0, 10);
	
	// �׷��Ƚ� �ܰ迡���� �������� ��������� �ϳ��� ����� ���� srv�� �����,
	// ��ǻƮ �ܰ迡���� �� ���� �о�� �ϹǷ� �ʸ����� uav�� �����.
	for (int i = 0; i < NUM_SHADOW_MAP; ++i) {
		Shader::CreateComputeUnorderedAccessView(pDevice, pShadowMap, i);
	}

	// ��ǻƮ ���̴� ��Ʈ�ñ״�ó 5, 6���� �̸� ������ ������, �ش� �����ӿ� �׷��� �����츦 ����

	for (auto& pBakedShadowMap : pBakedShadowMaps) {
		Shader::CreateComputeShaderResourceViews(pDevice, pBakedShadowMap, 0, 5);
	}
	
	Shader::CreateComputeShaderResourceViews(pDevice, pDynamicShadowMap, 0, 6);

	// ����Ʈ ���ۿ� ���� srv�� �����. 
	Shader::CreateComputeShaderResourceViews(pDevice, pPostBuffer, 0, 0);

	//// dest buffer���� �׷���, ��ǻƮ ���� �ϳ��� srv�� �����.
	//Shader::CreateShaderResourceViews(pDevice, pDestBuffer, 0, 16);

	// ��ǻƮ ���۴� ��ǻƮ ���̴����� �� uav�� �ϳ� ������ش�.
	Shader::CreateShaderResourceViews(pDevice, pComputeBuffer, 0, 16);
	Shader::CreateComputeUnorderedAccessView(pDevice, pComputeBuffer, 0);

	// 
	Shader::CreateShaderResourceViews(pDevice, pWireFrameMap, 0, 17);
	Shader::CreateComputeShaderResourceViews(pDevice, pWireFrameMap, 0, 2);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	// ����Ÿ�ٺ� �ּҸ� ���� ���� �Ҵ�
	GBufferCPUDescriptorHandles.resize(NUM_G_BUFFER);
	dynamicShadowMapCPUDescriptorHandles.resize(NUM_SHADOW_MAP);

	for (UINT i = 0; i < NUM_G_BUFFER; i++)
	{
		d3dRenderTargetViewDesc.Format = format[i];
		// i��° ���ҽ��� ���� ����Ÿ�ٺ並 ������ �ּҸ� ����
		ID3D12Resource* pd3dTextureResource = pGBuffer->GetResource(i).Get();

		// �⺻ ����Ÿ�ٺ� ���� �ּҿ� �ٷ� �����.
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvDescriptorHeapCurrentHandle);
		GBufferCPUDescriptorHandles[i] = rtvDescriptorHeapCurrentHandle;

		rtvDescriptorHeapCurrentHandle.ptr += ::rtvDescriptorIncrementSize;
	}

	for (UINT i = 0; i < NUM_SHADOW_MAP; i++)
	{
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		// i��° ���ҽ��� ���� ����Ÿ�ٺ並 ������ �ּҸ� ����
		ID3D12Resource* pd3dTextureResource = pDynamicShadowMap->GetResource(i).Get();

		// g ���� �ּҿ� �̾ �����.
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvDescriptorHeapCurrentHandle);
		dynamicShadowMapCPUDescriptorHandles[i] = rtvDescriptorHeapCurrentHandle;

		rtvDescriptorHeapCurrentHandle.ptr += ::rtvDescriptorIncrementSize;
	}

	{
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ID3D12Resource* pd3dTextureResource = pPostBuffer->GetResource(0).Get();
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvDescriptorHeapCurrentHandle);
		postBufferCPUDescriptorHandle = rtvDescriptorHeapCurrentHandle;

		rtvDescriptorHeapCurrentHandle.ptr += ::rtvDescriptorIncrementSize;
	}

	for (int i = 0; i < pBakedShadowMaps.size(); ++i) {

		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		// i��° ���ҽ��� ���� ����Ÿ�ٺ並 ������ �ּҸ� ����
		ID3D12Resource* pd3dTextureResource = pBakedShadowMaps[i]->GetResource(0).Get();

		// ���� ���� ���� ���۵� ������ �����.
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvDescriptorHeapCurrentHandle);
		bakedShadowMapCPUDescriptorHandles[i] = rtvDescriptorHeapCurrentHandle;

		rtvDescriptorHeapCurrentHandle.ptr += ::rtvDescriptorIncrementSize;
	}

	{
		d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ID3D12Resource* pd3dTextureResource = pWireFrameMap->GetResource(0).Get();
		if (pd3dTextureResource) pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvDescriptorHeapCurrentHandle);
		wireFrameMapCPUDescriptorHandle = rtvDescriptorHeapCurrentHandle;

		rtvDescriptorHeapCurrentHandle .ptr += ::rtvDescriptorIncrementSize;
	}

}

const shared_ptr<Scene>& GameFramework::GetCurrentScene() const {
	//if (!pScenes.empty()) {
	return pScenes.top();
	//}
}

void GameFramework::BakeShadowMap() {
	
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	shared_ptr<PlayScene> pScene = static_pointer_cast<PlayScene>(pScenes.top());
	pScene->BakeShadowMap(pCommandList, dsvCPUDescriptorHandle);
	//��� ����Ʈ�� ���� ���·� �����. 
	HRESULT hResult = pCommandList->Close();
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�. 
	vector<ComPtr<ID3D12CommandList>> pCommandLists = { pCommandList.Get() };

	pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());

	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���. 
	WaitForGpuComplete();

	//	����ü���� ������Ʈ�Ѵ�.
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	HRESULT hr = pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// ���� ���������� �̵�, (���� ���۷� �̵�)
	MoveToNextFrame();
}

void GameFramework::RenderDynamicShadowMap() {

	auto pScene = dynamic_pointer_cast<PlayScene>(pScenes.top());
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	D3D12_CPU_DESCRIPTOR_HANDLE* rtvShadowCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[1];

	for (int i = 0; i < NUM_SHADOW_MAP; ++i)
	{
		// ����Ÿ������ �����ϰ� ������������ ������ �����.
		SynchronizeResourceTransition(pCommandList.Get(), pDynamicShadowMap->GetResource(i).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
		rtvShadowCPUDescriptorHandles[0].ptr = dynamicShadowMapCPUDescriptorHandles[i].ptr;

		pCommandList->ClearRenderTargetView(rtvShadowCPUDescriptorHandles[0], pClearColor, 0, NULL);
	}

	// SetPipelineState Ƚ���� ���̱� ����
	for (int i = 0; i < NUM_SHADOW_MAP; ++i)
	{
		// i��° ��������� ����Ÿ������ �����Ѵ�.
		rtvShadowCPUDescriptorHandles[0].ptr = dynamicShadowMapCPUDescriptorHandles[i].ptr;
		pCommandList->OMSetRenderTargets(1, rtvShadowCPUDescriptorHandles, TRUE, NULL);
		pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		// i��° ���� ���� ������� �������� �Ѵ�.
		pScene->RenderShadowMap(pCommandList, i);
	}

	for (int i = 0; i < NUM_SHADOW_MAP; ++i)
	{
		SynchronizeResourceTransition(pCommandList.Get(), pDynamicShadowMap->GetResource(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
	delete[] rtvShadowCPUDescriptorHandles;
}

void GameFramework::MergeShadowMap() {
	// �̸� �׷����� ������ʰ� ����
	Shader::SetComputeDescriptorHeap(pCommandList);
	auto pScene = dynamic_pointer_cast<PlayScene>(pScenes.top());

	auto pShader = static_pointer_cast<ShadowComputeShader>(GetShader("ShadowComputeShader"));
	pShader->PrepareRender(pCommandList);

	for (int i = 0; i < NUM_SHADOW_MAP; ++i) {
		// �ش� �ε����� ���� �̸� �������� ������ ���� ����
		pScene->GetLight(i)->pBakedShadowMap->UpdateComputeShaderVariable(pCommandList, 5, -1);

		// �ش� �ε����� ���� ���� ���� ������ ���� ����
		pDynamicShadowMap->UpdateComputeShaderVariable(pCommandList, 6, -1, i);

		// �� ������ ���� ������ ����� �� RWTexture�� uav 2���� ����
		pShadowMap->UpdateComputeShaderVariable(pCommandList, -1, 2, i);

		// �� ��������� ����
		pShader->Dispatch(pCommandList);
	}

	// �ٽ� �׷��Ƚ� ��ũ���� ���� ����
	Shader::SetDescriptorHeap(pCommandList);
	pShadowMap->UpdateShaderVariable(pCommandList);
}

void GameFramework::RenderGBuffer() {
	// �̸� �׸� ������ �ڵ�
	float timeElapsed = gameTimer.GetTimeElapsed();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	D3D12_CPU_DESCRIPTOR_HANDLE* rtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[NUM_G_BUFFER];

	for (int i = 0; i < NUM_G_BUFFER; ++i)
		SynchronizeResourceTransition(pCommandList.Get(), pGBuffer->GetResource(i).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	for (int i = 0; i < NUM_G_BUFFER; ++i) {
		rtvCPUDescriptorHandles[i].ptr = GBufferCPUDescriptorHandles[i].ptr;
		pCommandList->ClearRenderTargetView(GBufferCPUDescriptorHandles[i], pClearColor, 0, NULL);
	}

	// G Buffer �� ���� ó������ ���� �׸��� ���� ���۵��� ����Ÿ�ٿ� Set�Ѵ�. 
	pCommandList->OMSetRenderTargets(NUM_G_BUFFER, rtvCPUDescriptorHandles, FALSE, &dsvCPUDescriptorHandle);
	pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// �ش� ���� ���� G Buffer �� ������ ������ ���� �׸���.
	if (!pScenes.empty()) {
		pScenes.top()->PreRender(pCommandList, timeElapsed);
	}

	for (int i = 0; i < NUM_G_BUFFER; ++i)
		SynchronizeResourceTransition(pCommandList.Get(), pGBuffer->GetResource(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

	delete[] rtvCPUDescriptorHandles;
}

void GameFramework::RenderWireFrame() {
	// ���̾������� ������
	float timeElapsed = gameTimer.GetTimeElapsed();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	SynchronizeResourceTransition(pCommandList.Get(), pWireFrameMap->GetResource(0).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	pCommandList->ClearRenderTargetView(wireFrameMapCPUDescriptorHandle, pClearColor, 0, NULL);

	pCommandList->OMSetRenderTargets(1, &wireFrameMapCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
	pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	if (!pScenes.empty()) {
		shared_ptr<PlayScene> pPlayScene = dynamic_pointer_cast<PlayScene>(pScenes.top());
		if (pPlayScene)
			pPlayScene->WireFrameRender(pCommandList, timeElapsed);
	}
	SynchronizeResourceTransition(pCommandList.Get(), pGBuffer->GetResource(0).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

}

void GameFramework::LightingAndComputeBlur() {
	// ����Ʈ ���ۿ� ����ó���� ���� ���� �׸���.
	float timeElapsed = gameTimer.GetTimeElapsed();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	pCommandList->OMSetRenderTargets(1, &postBufferCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
	pCommandList->ClearRenderTargetView(postBufferCPUDescriptorHandle, pClearColor, 0, NULL);
	pScenes.top()->LightingRender(pCommandList, timeElapsed);
	pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// ����Ʈ ���ۿ� ���� �� ó���� �����Ѵ�.
	Shader::SetComputeDescriptorHeap(pCommandList);
	static_pointer_cast<BlurComputeShader>(GetShader("BlurComputeShader"))->Dispatch(pCommandList);

	// �� ó���� ���� �ؽ�ó�� �ĸ���ۿ� �״�� �׸���.
	Shader::SetDescriptorHeap(pCommandList);
}

void GameFramework::FrameAdvance() {

	gameTimer.Tick(0.0f);

	// ��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.
	HRESULT hResult = pCommandAllocator->Reset();
	hResult = pCommandList->Reset(pCommandAllocator.Get(), NULL);
	vector<ComPtr<ID3D12CommandList>>  pCommandLists;

	float timeElapsed = gameTimer.GetTimeElapsed();

	float pClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// �Է��� ���� �� �÷��̾��� �������� �����Ѵ�.

	ProcessInput();

	if (!pScenes.empty()) {	// �� ����(�ִϸ���Ʈ). ������ �� �� ���ҿ� ���� ����
		// ����� �̵�, ȸ������ ���� �浹üũ�� �����غ� �� ������ �÷��̾ �����δ�.
		pScenes.top()->AnimateObjects(pScenes.top()->CheckCollision(timeElapsed), timeElapsed, pDevice, pCommandList);
	}

	// ������� �� ������Ʈ���� ���̴��� ������Ʈ ���Ϳ��� ���ش�.
	Shader::UpdateShadersObject();

	// ���� ���� Ÿ�ٿ� ���� Present�� �����⸦ ��ٸ�.  (PRESENT = ������Ʈ ����, RENDER_TARGET = ���� Ÿ�� ����
	::SynchronizeResourceTransition(pCommandList, pRenderTargetBuffers[swapChainBufferCurrentIndex].Get(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// ��Ʈ �ñ״�ó�� Set
	pCommandList->SetGraphicsRootSignature(pRootSignature.Get());
	pCommandList->SetComputeRootSignature(pComputeRootSignature.Get());

	
	// �ĸ� ������ �ڵ�
	D3D12_CPU_DESCRIPTOR_HANDLE swapChainDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	swapChainDescriptorHandle.ptr += (rtvDescriptorIncrementSize * swapChainBufferCurrentIndex);

	// ���� ���ٽ� ������ �ڵ�
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	Shader::SetDescriptorHeap(pCommandList);

	static bool baked = false;

	// ù PlayScene ���� �� ���� ������� ���� ����
	if (!baked && !pScenes.empty()) {
		auto pScene = dynamic_pointer_cast<PlayScene>(pScenes.top());
		if (pScene) {
			BakeShadowMap();
			baked = true;
			return;
		}
	}

	if (!pScenes.empty()) {
		auto pScene = dynamic_pointer_cast<PlayScene>(pScenes.top());
		if (pScene) {

			pScene->UpdateLightShaderVariables(pCommandList);
			pScene->UpdateCameraShaderVariables(pCommandList);

			// ���� �׸��� �� ������	
			RenderDynamicShadowMap();
			
			// �̸� �׷����� ������ʰ� ����
			MergeShadowMap();
			
			// G Buffer ������
			RenderGBuffer();

			// ���̾� ������ ������
			RenderWireFrame();

			// ��ó�� �� ��, ���̴� ȿ�� ����
			LightingAndComputeBlur();

		}
		// ���� ����� �ĸ���ۿ� ���
		pCommandList->OMSetRenderTargets(1, &swapChainDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
		pCommandList->ClearRenderTargetView(swapChainDescriptorHandle, pClearColor, 0, NULL);
		pScenes.top()->Render(pCommandList, timeElapsed);
	}


	// ���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���.
	::SynchronizeResourceTransition(pCommandList, pRenderTargetBuffers[swapChainBufferCurrentIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	 
	//��� ����Ʈ�� ���� ���·� �����. 
	hResult = pCommandList->Close();
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�. 
	pCommandLists = { pCommandList.Get() };

	pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());

	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���. 
	WaitForGpuComplete();

	if (!pScenes.empty()) {
		pScenes.top()->PostRender();
	}

	//	����ü���� ������Ʈ�Ѵ�.
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	HRESULT hr = pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// ���� ���������� �̵�, (���� ���۷� �̵�)
	MoveToNextFrame();
	
	// FPS ǥ��
	wstring titleString = L"FPS : " + to_wstring(gameTimer.GetFPS());
	SetWindowText(windowHandle, (LPCWSTR)titleString.c_str());
}

void GameFramework::WaitForGpuComplete() {
	UINT64 fenceValue = ++fenceValues[swapChainBufferCurrentIndex]; // ���� ������ �潺���� ����
	HRESULT hResult = pCommandQueue->Signal(pFence.Get(), fenceValue);
	if (pFence->GetCompletedValue() < fenceValue) {
		hResult = pFence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);					// GPU�� ó���� ������ ���� ��ٸ�
	}
}

void GameFramework::MoveToNextFrame() {
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex(); // ���� �ĸ� ���۷� ����
	WaitForGpuComplete();
}

void GameFramework::ChangeSwapChainState() {
	return;
	WaitForGpuComplete();
	BOOL fullScreenState;

	// ���� ��� ����

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = clientWidth;
	dxgiTargetParameters.Height = clientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;


	if (!fullScreenState) {
		pDxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
		//pDxgiSwapChain->SetFullscreenState(~fullScreenState, NULL);
	}
	else {
		//pDxgiSwapChain->SetFullscreenState(~fullScreenState, NULL);
		pDxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	}


	for (int i = 0; i < nSwapChainBuffer; i++) {
		if (pRenderTargetBuffers[i]) {
			pRenderTargetBuffers[i].Reset();
		}
	}
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	pDxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	dxgiSwapChainDesc.Flags = 0;
	pDxgiSwapChain->ResizeBuffers(nSwapChainBuffer, clientWidth,
		clientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();

}

void GameFramework::ProcessInput() {

	static array<UCHAR, 256> keysBuffers;
	static array<bool, 256> keysDownStateBuffers;	// Ű�� ������������ Ȯ���Ѵ� ����, 
	static array<bool, 256> keysDownBuffers;		// ���� Ű�� ���� �������� �����ϴ� ����
	static array<bool, 256> keysUpBuffers;		// ���� Ű�� �ȴ��� �������� �����ϴ� ����
	bool processedByScene = false;

	keysDownBuffers.fill(false);
	keysUpBuffers.fill(false);
	if (GetKeyboardState((PBYTE)keysBuffers.data())) {	// Ű����� ���� �Էµ����͸� ������
		for (int i = 0; i < 256; ++i) {
			if ((keysBuffers[i] & 0xF0) && !keysDownStateBuffers[i]) {	// Ű�� ������ ����
				keysDownStateBuffers[i] = true;
				keysDownBuffers[i] = true;
			}
			else if (!(keysBuffers[i] & 0xF0) && keysDownStateBuffers[i]) {	// Ű�� ���� ����.
				keysDownStateBuffers[i] = false;
				keysUpBuffers[i] = true;
			}
		}
		
		if (keysDownBuffers['F']) {
			ChangeSwapChainState();
		}
		// �Ͻ�����
		if (keysDownBuffers['P']) {
			//PushScene(make_shared<Scene>("pause"));
		}
		// �����
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

		// ���� Ű�����Է� ó��
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

shared_ptr<Texture> GameFramework::GetDynamicShadowMap() const {
	return pDynamicShadowMap;
}

shared_ptr<Texture> GameFramework::GetBakedShadowMap(int _index) const
{
	return pBakedShadowMaps[_index];
}

D3D12_CPU_DESCRIPTOR_HANDLE GameFramework::GetBakedShadowMapHandle(int _index) const
{
	return bakedShadowMapCPUDescriptorHandles[_index];
}

shared_ptr<Texture> GameFramework::GetWireFrameMap() const {
	return pWireFrameMap;
}

shared_ptr<Texture> GameFramework::GetDestBuffer() const {
	return pDestBuffer;
}



void GameFramework::CreateShaderVariables() {
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256�� ���

	pcbFrameworkInfo = ::CreateBufferResource(pDevice.Get(), pCommandList.Get(), NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);

	pcbFrameworkInfo->Map(0, NULL, (void**)&pcbMappedFrameworkInfo);
}
void GameFramework::UpdateShaderVariables() {
	float tempTimeElapsed = gameTimer.GetTimeElapsed();
	static float g_Time = 0;
	g_Time += 0.0002f;
	pcbMappedFrameworkInfo->currentTime = (float)g_Time;
	pcbMappedFrameworkInfo->elapsedTime = (float)gameTimer.GetTimeElapsed();

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pcbFrameworkInfo->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(15, d3dGpuVirtualAddress);
}

