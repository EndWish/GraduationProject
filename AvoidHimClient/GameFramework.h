#pragma once
#include "Timer.h"
#include "Scene.h"
#include "Sound.h"

struct CB_FRAMEWORK_INFO {
	float currentTime;
	float elapsedTime;
	UINT width;
	UINT height;
};

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

	// 컴퓨트 쉐이더를 위한 루트 시그니처
	ComPtr<ID3D12RootSignature> pComputeRootSignature;

	// 게임 타이머
	Timer gameTimer;

	// 이전 프레임에서의 마우스 커서 위치를 저장하는 변수
	POINT oldCursorPos;

	HitBoxMesh hitBoxMesh;
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

	// 사운드 매니저
	SoundManager pSoundManager;

	// 애니메이션 매니저 ? 
	// 씬
	stack<shared_ptr<Scene>> pScenes;

	// G Buffer
	shared_ptr<Texture> pGBuffer;
	// G Buffer의 렌더타겟뷰 주소
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> GBufferCPUDescriptorHandles;
	
	// 
	vector < shared_ptr<Texture>>pBakedShadowMaps;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> bakedShadowMapCPUDescriptorHandles;


	// 매 프레임 그려지는 쉐도우 맵
	shared_ptr<Texture> pDynamicShadowMap;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> dynamicShadowMapCPUDescriptorHandles;
	
	// 쉐도우 맵 = dynamic + baked
	shared_ptr<Texture> pShadowMap;

	// 빛 처리를 마친 화면 버퍼
	shared_ptr<Texture> pPostBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE postBufferCPUDescriptorHandle;

	// 컴퓨트 쉐이더의 계산 결과를 출력할 버퍼
	shared_ptr<Texture> pComputeBuffer;

	// 쉐도우 맵
	shared_ptr<Texture> pWireFrameMap;
	D3D12_CPU_DESCRIPTOR_HANDLE wireFrameMapCPUDescriptorHandle;

	// 컴퓨트 쉐이더의 계산 결과를 복사할 버퍼
	shared_ptr<Texture> pDestBuffer;

	// 쉐이더 변수
	ComPtr<ID3D12Resource> pcbFrameworkInfo;
	shared_ptr<CB_FRAMEWORK_INFO> pcbMappedFrameworkInfo;


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
	void CreateComputeRootSignature();	// 루트 시그니쳐 생성
	
	// 쉐이더 생성
	bool InitShader();
	void InitBuffer();
	
	// Frame Advance

	void BakeShadowMap();
	void RenderDynamicShadowMap();
	void MergeShadowMap();
	void RenderGBuffer();
	void RenderWireFrame();
	void LightingAndComputeBlur();
	// get set 함수


public:

	void NoticeCloseToServer();
	void ProcessMouseInput(UINT _type, XMFLOAT2 _pos);
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	pair<int, int> GetClientSize();
	POINT GetOldCursorPos() { return oldCursorPos; };
	HitBoxMesh& GetHitBoxMesh() { return hitBoxMesh; };
	
	const shared_ptr<Scene>& GetCurrentScene() const;

	void InitOldCursor();


	TextureManager& GetTextureManager();
	GameObjectManager& GetGameObjectManager();
	SoundManager& GetSoundManager();

	shared_ptr<Shader> GetShader(const string& _name);
	UINT GetCurrentSwapChainIndex() const { return swapChainBufferCurrentIndex; };

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

	shared_ptr<Texture> GetGBuffer() const;
	shared_ptr<Texture> GetShadowMap() const;
	shared_ptr<Texture> GetPostBuffer() const;
	shared_ptr<Texture> GetComputeBuffer() const;
	shared_ptr<Texture> GetDynamicShadowMap() const;
	shared_ptr<Texture> GetBakedShadowMap(int _index) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetBakedShadowMapHandle(int _index) const;
	shared_ptr<Texture> GetWireFrameMap() const;
	shared_ptr<Texture> GetDestBuffer() const;



	void CreateShaderVariables();
	void UpdateShaderVariables();
	int GetFPS();
};
