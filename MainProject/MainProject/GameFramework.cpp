#include "stdafx.h"
#include "GameFramework.h"

/// ���� ������
unique_ptr<GameFramework> GameFramework::s_pInstance;

/// ���� �Լ���
// ���� �Լ� �� �Ҹ� �Լ�
bool GameFramework::Create(HINSTANCE hInstance, HWND hMainWnd) 
{
	if (!s_pInstance) {
		// GameGramwork �� �������� ������
		s_pInstance.reset(new GameFramework());	// �����Ѵ�.
		GameFramework& gameFramework = *s_pInstance;

		gameFramework.m_hInstance = hInstance;
		gameFramework.m_hWnd = hMainWnd;

		gameFramework.CreateDirect3dDevice();	// ������� ����̽��� �����ؾ� ��� ��⿭�̳� ������ �� ���� ������ �� �ִ�.
		gameFramework.CreateCommandQueueAndList();
		gameFramework.CreateRtvAndDsvDescriptorHeaps();
		gameFramework.CreateSwapChain();	// DxgiFactory, CommandQueue, RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
		gameFramework.CreateDepthStencilView();
		gameFramework.CreateGraphicsRootSignature();
		
		Shader::instance.CreateShader(gameFramework.m_pDevice, gameFramework.m_pRootSignature);	// �ӽ÷� ���̴��� �����غ� [����]

		// ������ �� ���� [����]
		shared_ptr<Scene> startScene = make_shared<Scene>();
		gameFramework.PushScene(startScene);

		gameFramework.m_gameTimer.Reset();	// Ÿ�̸� ����

		return true;
	}

#ifdef DEBUG
	cout << "GameFramework::OnCreate() : �̹� ������ GameFramework�� �ѹ��� ������� �Ͽ����ϴ�.\n";
#endif // DEBUG

	return false;
}
void GameFramework::Destroy() 
{
	if (s_pInstance) {
		GameFramework& gameFramework = *s_pInstance;

		// ������ Destroy �Ѵ�[����]
		::CloseHandle(gameFramework.m_fenceEvent);
	}

#ifdef DEBUG
	cout << "GameFramework::OnDestroy() : ������ GameFramework �� ���µ� �Ҹ��Ű���� �Ͽ����ϴ�.\n";
#endif // DEBUG
}
GameFramework& GameFramework::Instance() 
{
#ifdef DEBUG
	if (!s_pInstance)
		cout << "GameFramework::Instance() : Create���� ���� ���¿��� ȣ���Ͽ����ϴ�.\n";
#endif // DEBUG

	return *s_pInstance;
}

// �������� ��� �Լ���
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

	//------------------------------------���Ӱ��� ����-------------------------------------
	// Ÿ�̸�
	m_gameTimer = GameTimer();

}
GameFramework::~GameFramework() 
{

}

// ���� ó�� �������ؾ� �ϴ� �͵� (OnCreate()�� ���Եȴ�.)
void GameFramework::CreateDirect3dDevice() 
{

	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pDxgiFactory );	// m_pDxgiFactory.GetAddressOf()�� &m_pDxgiFactory �� ����.

	ComPtr<IDXGIAdapter1> pAdapter = NULL;	// �˾Ƽ� �Ҹ��

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDxgiFactory->EnumAdapters1(i, &pAdapter); i++) {	// i��° ���(�׷��� ī��)�� ���� �����͸� pD3dAdapter������ ����
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pAdapter->GetDesc1(&dxgiAdapterDesc);	// ������� ������ �����´�.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // ����Ͱ� ����Ʈ�����̸� �ǳʶڴ�.
			continue;	

		if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pDevice)))	// �մ��� ����Ͱ� ������ �װ����� ����̽��� �����Ѵ�.
			break;
	}

	if (!pAdapter) {
		m_pDxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)pAdapter.GetAddressOf());
		hResult = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pDevice );
	}

	// ���߻��ø� ����
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

	// ��� ��⿭ ����
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	::ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pDevice->CreateCommandQueue(&commandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pCommandQueue);

	// ��� �Ҵ��� ����
	hResult = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pCommandAllocator);

	// ��� ��� ���� - CPU�� ���� ��� ����� ��� ��⿭�� �����ߴٰ� �ؼ� ��� ����Ǵ� ���� �ƴԿ� ����! (CPU�� GPU�� ���������� ���ư��� ������)
	hResult = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pCommandList);
	hResult = m_pCommandList->Close();
	 
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
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	::ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	descriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// ���� ���� Ÿ���� ����, �ĸ� 2�� ���
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pRtvDescriptorHeap);
	m_rtvDescriptorIncrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// ���� Ÿ�� �並 ���鶧 ����

	// ����-���ٽ��� ������ �� ����
	descriptorHeapDesc.NumDescriptors = 1;	// ����-���ٽ��� 1�� ���
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pDsvDescriptorHeap);
	m_dsvDescriptorIncrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ��¿� ���۷� ���ڴٰ� �����ϴ°�.		�б�����, ���̴��� �Է����� ���, ����, ������ȭ ���� ���� ���°� �ִ�.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// ���� ���ۿ� �ĸ���۸� ��ü�ϰ� ���� ������ ���(���� ���� ������ �ӵ��� ������)�Ѵ�.
	dxgiSwapChainDesc.OutputWindow = m_hWnd;	// ����� ������
	dxgiSwapChainDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;	// â��带 ������?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pDxgiFactory->CreateSwapChain(m_pCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_pDxgiSwapChain.GetAddressOf() );

	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pDxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	CreateRenderTargetViews();	// RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
}
void GameFramework::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = m_pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	// ������ ���� ���� �����ּҸ� �����´�.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppRenderTargetBuffers[i]);	// ����Ÿ�� ���۸� �����Ѵ�.
		m_pDevice->CreateRenderTargetView(m_ppRenderTargetBuffers[i].Get(), NULL, rtvCPUDescriptorHandle);	// ����Ÿ�� �並 ������ ���� ����(����) (��==������?)
		rtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;	// ������ �ּҷ� �̵�
	}
}
void GameFramework::CreateDepthStencilView()
{
	// ����-���ٽ� ����(���ҽ�)�� ����� ���� ������ �Է�
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

	// ����-���ٽ� ����(���ҽ�) ����
	m_pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&m_pDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	::ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, dsvCPUDescriptorHandle);	// ����-���ٽ� �並 ������ ���� ����(����) (��==������?)
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

/// ��� �Լ���
// Get Set �Լ�
Scene& GameFramework::GetCurrentSceneRef()
{
#ifdef DEBUG
	if (m_pScenes.empty()) {
		cout << "GameFramework::GetCurrentScene() : ���� �������� �ʴµ� ���� �������� ��!\n";
	}
#endif // DEBUG

	return *m_pScenes.top();
}
pair<int, int> GameFramework::GetWindowClientSize() const 
{
	return make_pair(m_windowClientWidth, m_windowClientHeight);
}

// ���� ���������� �����ϴ� �Լ�
void GameFramework::FrameAdvance() 
{
	m_gameTimer.Tick(60.0f);

	// �Է��� ó���Ѵ�. [����]
	// ���� �����Ų��. [����]
	
	
	//----------------------------------- ������ �Ѵ� ----------------------------------
	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�. 
	HRESULT hResult = m_pCommandAllocator->Reset();
	hResult = m_pCommandList->Reset(m_pCommandAllocator.Get(), NULL);

	/*���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���. ������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ����
	(D3D12_RESOURCE_STATE_PRESENT)���� ���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� ��
	�� ���̴�.*/
	D3D12_RESOURCE_BARRIER resourceBarrier;
	::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = m_ppRenderTargetBuffers[m_swapChainBufferCurrentIndex].Get();
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &resourceBarrier);

	//������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = m_pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCPUDescriptorHandle.ptr += (m_swapChainBufferCurrentIndex * m_rtvDescriptorIncrementSize);

	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�. 
	m_pCommandList->OMSetRenderTargets(1, &rtvCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);

	//���ϴ� �������� ���� Ÿ��(��)�� �����. 
	float pClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };	/*Colors::Azure*/
	m_pCommandList->ClearRenderTargetView(rtvCPUDescriptorHandle, pClearColor, 0, NULL);

	//���ϴ� ������ ����-���ٽ�(��)�� �����. 
	m_pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//�׷��� ��Ʈ �ñ׳��ĸ� ���������ο� ����(����)�Ѵ�.
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	//�� ������	(Scene���� ī�޶�, �÷��̾ �����Ѵ�.)
	//GetCurrentSceneRef().Render(m_pCommandList);	[����]

	/*���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ��
	�� ���´� ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.*/
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pCommandList->ResourceBarrier(1, &resourceBarrier);

	//��� ����Ʈ�� ���� ���·� �����. 
	hResult = m_pCommandList->Close();

	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�. 
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���. 
	WaitForGpuComplete();

	/*����ü���� ������Ʈ�Ѵ�. ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������
	������۷� �Ű����� ���� Ÿ�� �ε����� �ٲ� ���̴�.*/
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// ���� ���������� �̵�
	MoveToNextFrame();

	// �޴��� �̸��� FPS�� ����
	wstring ws = L"FPS : " + to_wstring(m_gameTimer.GetFPS());
	::SetWindowText(m_hWnd, (LPCWSTR)ws.c_str());	// FPS�� ���� ���� ���� ��� ���ɸ�

}
//GPU�� ����ȭ�� ���� ���
void GameFramework::WaitForGpuComplete() 
{
	//CPU �潺�� ���� ������Ű��,
	//GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�. 
	UINT64 fenceValue = ++m_fenceValues[m_swapChainBufferCurrentIndex];
	HRESULT hResult = m_pCommandQueue->Signal(m_pFence.Get(), fenceValue);
	if (m_pFence->GetCompletedValue() < fenceValue) {
		//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���. 
		hResult = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
// ���� �ĸ���۷� �ٲٰ� WaitForGpuComplete() ����
void GameFramework::MoveToNextFrame() 
{
	m_swapChainBufferCurrentIndex = m_pDxgiSwapChain->GetCurrentBackBufferIndex();

	WaitForGpuComplete();
}

// �� ���� �Լ���
void GameFramework::PushScene(const shared_ptr<Scene>& pScene)
{
	// ���� ���� �Ͻ�����
	if (!m_pScenes.empty())
		m_pScenes.top()->Pause();

	// ���ο� ������ ��ȯ
	m_pScenes.push(pScene);

	// ���ο� �� �ʱ�ȭ
	m_pScenes.top()->Enter();
	
}
void GameFramework::PopScene() 
{
#ifdef DEBUG
	if (m_pScenes.empty()) {
		cout << "GameFramework::PopScene() : ���� �������� �ʴµ� ���� �����Ϸ��� ��!\n";
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
