#pragma once
#include "Timer.h"
#include "Scene.h"

class GameFramework {
	// 정적 변수 
private:
	static unique_ptr<GameFramework> spInstance;	// 고유 프레임워크 

public:
	static void Create(HINSTANCE _hInstance, HWND _hMainWnd);
	static void Destroy();

	// 프레임워크 인스턴스를 전역으로 접근 하기 위한 함수
	static GameFramework& Instance();


private:
	HINSTANCE instanceHandle;
	HWND windowHandle;

	ComPtr<IDXGIFactory4> pDxgiFactory;										// DXGI 팩토리 인터페이스에 대한 포인터
	ComPtr<IDXGISwapChain3> pDxgiSwapChain;									// 스왑체인 인터페이스
	ComPtr<ID3D12Device> pDevice;

	bool msaa4xEnable;
	UINT msaa4xLevel;

	static const UINT nSwapChainBuffer = 2;									// 스왑 체인 버퍼 개수
	UINT swapChainBufferCurrentIndex;										// 현재 그릴 스왑체인의 후면버퍼 인덱스 

	// 렌더타겟, 깊이-스텐실 버퍼
	array<ComPtr<ID3D12Resource>, nSwapChainBuffer> pRenderTargetBuffers;	// 렌더 타겟 버퍼(후면) 포인터를 담는 배열
	array<D3D12_CPU_DESCRIPTOR_HANDLE, nSwapChainBuffer> rtvCPUDescriptorHandles;
	ComPtr<ID3D12DescriptorHeap> pRtvDescriptorHeap;						// 렌더타겟버퍼의 서술자 힙. 
	UINT rtvDescriptorIncrementSize;										// 렌더타겟 서술자의 크기

	ComPtr<ID3D12Resource> pDepthStencilBuffer;								// 깊이-스텐실 버퍼 포인터를 담는 배열
	ComPtr<ID3D12DescriptorHeap> pDsvDescriptorHeap;						// 깊이-스텐실 버퍼의 서술자 힙. 
	UINT dsvDescriptorIncrementSize;										// 깊이-스텐실 버퍼의 서술자의 크기


	// 명령 큐, 명령 할당자, 명령 리스트의 인터페이스 포인터
	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;

	// 펜스
	ComPtr<ID3D12Fence> pFence;
	HANDLE fenceEvent;
	array<UINT64, nSwapChainBuffer> fenceValues;

	// 루트 시그니처
	ComPtr<ID3D12RootSignature> pRootSignature;

	// 게임 타이머
	Timer gameTimer;


	// 이전 프레임에서의 마우스 커서 위치를 저장하는 변수
	POINT oldCursorPos;

	// 현재 왼쪽 마우스가 눌리고 있는지를 판단
	bool isClick;

	///// 아래는 Instance()에서 다른 함수로 접근 가능한 변수들
	
	int clientWidth;
	int clientHeight;
	
	// 마테리얼 매니저
	TextureManager textureManager;
	
	// 게임 오브젝트 매니저
	GameObjectManager gameObjectManager;
	
	ShaderManager shaderManager;
	// 애니메이션 매니저 ? 
	// 씬
	stack<shared_ptr<Scene>> pScenes;

	// 생성, 소멸자
private:
	GameFramework();
public:
	~GameFramework();

	// 생성시 초기화용 함수
protected:
	void CreateDirect3DDevice();		// dxgiFactory를 생성하고, 그것을 이용하여 디바이스를 생성
	void CreateCommandQueueList();		// 커맨드 큐 리스트 생성
	void CreateRtvAndDsvDescriptorHeaps();	// 서술자 힙 생성
	void CreateSwapChain();				// 스왑체인 생성
	void CreateRenderTargetViews();		// 렌더타겟뷰 생성 후 서술자 힙에 적재
	void CreateDepthStencilView();
	void CreateGraphicsRootSignature();	// 루트 시그니쳐 생성
	
	// 쉐이더 생성
	bool InitShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);

	// get set 함수
public:

	void NoticeCloseToServer();
	void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	pair<int, int> GetClientSize();
	POINT GetOldCursorPos() { return oldCursorPos; };

	// 루트 시그니처 Get
	ComPtr<ID3D12RootSignature> GetRootSignature() { return pRootSignature; }
	const shared_ptr<Scene>& GetCurrentScene() const;

	void InitOldCursor();

	TextureManager& GetTextureManager();
	GameObjectManager& GetGameObjectManager();
	shared_ptr<Shader> GetShader(const string& _name);
	UINT GetCurrentSwapChainIndex() { return swapChainBufferCurrentIndex; };

	void FrameAdvance();
	void WaitForGpuComplete();			// GPU와 동기화하기 위한 대기
	void MoveToNextFrame();				// 다음 후면버퍼로 변경후 WaitForGpuComplete() 수행

	void ChangeSwapChainState();
	void ProcessInput();

	void PushScene(const shared_ptr<Scene>& _pScene);
	void LoadingScene(const shared_ptr<Scene>& _pScene);
	void PopScene();
	void ChangeScene(const shared_ptr<Scene>& _pScene);
	void ClearScene();
};
