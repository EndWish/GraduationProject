#pragma once

#include "GameTimer.h"
#include "Scenes.h"

class GameFramework {
private:	// 정적 변수▼
	static unique_ptr<GameFramework> s_pInstance;	// 싱글톤 변수, https://boycoding.tistory.com/109

public:		// 정적 함수▼
	// 생성시(초기화) 함수 및 소멸시 함수
	static bool Create(HINSTANCE hInstance, HWND hMainWnd);
	static void Destroy();

	// 전역 적으로 접근할 수 있는 함수
	static GameFramework& Instance();

private:	// 멤버 변수▼
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	//------------------------------------디바이스관련 변수-------------------------------------
	//클라이언트 해상도
	int m_windowClientWidth;
	int m_windowClientHeight;

	// DXGI(DirectX Graphics Infrastructure) : DirectX 그래픽을 런타임에 독립적인 저수준의 작업을 관리해줌, 그래픽을 위한 기본적이고 공통적인 프레이워크를 제공한다.
	// ComPtr는 Com객체에 대한 스마트 포인터 이다.
	ComPtr<IDXGIFactory4> m_pDxgiFactory;	// DXGI 팩토리 인터페이스에 대한 포인터이다. (어뎁터를 열거하는 역할, )
	ComPtr<IDXGISwapChain3> m_pDxgiSwapChain;	// 스왑 체인 인터페이스에 대한 포인터이다. 주로 디스플레이를 제어하기 위해 사용
	ComPtr<ID3D12Device> m_pDevice;	///Direct3D 디바이스 인터페이스에 대한 포인터이다. 주로 리소스를 생성하기 위하여 필요하다.

	//MSAA 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다
	bool m_msaa4xEnable;
	UINT m_msaa4xQualityLevels;

	static const UINT m_nSwapChainBuffers = 2;	//스왑 체인의 후면 버퍼의 개수이다.
	UINT m_swapChainBufferCurrentIndex; //현재 스왑 체인의 후면 버퍼 인덱스이다.

	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 
	ComPtr<ID3D12Resource> m_ppRenderTargetBuffers[m_nSwapChainBuffers];	//렌더타겟버퍼(후면버퍼) 포인터들
	ComPtr<ID3D12DescriptorHeap> m_pRtvDescriptorHeap;	//렌더타켓 서술자 힙 포인터, 서술자 힙은 서술자들을 담는 메모리이다. 서술자는 리소스가상 메모리를 관리해주는 매개체 https://ssinyoung.tistory.com/37 참고
	UINT m_rtvDescriptorIncrementSize;	//렌더 타겟 서술자 원소의 크기이다.
	
	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기이다.
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;	// 깊이-스텐실 버퍼는 렌더타겟버퍼와 다르게 1개만 있으면 된다.
	ComPtr<ID3D12DescriptorHeap> m_pDsvDescriptorHeap;
	UINT m_dsvDescriptorIncrementSize;

	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	
	//펜스 인터페이스 포인터, 이벤트 핸들, 펜스의 값 이다. 
	ComPtr<ID3D12Fence> m_pFence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[m_nSwapChainBuffers];

	//루트 시그니처 생성
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	
	//------------------------------------게임관련 변수-------------------------------------
	//
	GameTimer m_gameTimer;
	stack<shared_ptr<Scene>> m_pScenes;	// 씬들을 관리한다. top에 있는 씬이 현재 진행될 씬이다.

public:	// 생성관련 멤버 함수▼
	~GameFramework();	// 소멸자
private:
	GameFramework();	// 생성자

	// 생성시(초기화) 함수의 부분들
	void CreateDirect3dDevice();	// dxgiFactory를 생성하고, 그것을 이용하여 어댑터(그래픽 카드)를 살펴서 적절한 그래픽카드로 디바이스를 생성한다.
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();
	void CreateRenderTargetViews();		// GPU에서 읽을 수 있도록 렌더타겟 뷰를 생성 ( CreateSwapChain()과 화면 크기 전환할때 사용된다.  )
	void CreateDepthStencilView();	// 깊이-스텐실 버퍼(리소스)를 만들고 깊이-스텐실 뷰를 서술자 힙에 적제
	void CreateGraphicsRootSignature();	// 루트 시그니처를 생성한다.

public:		// 멤버 함수▼
	// Get Set 함수
	Scene& GetCurrentSceneRef();
	pair<int, int> GetWindowClientSize() const;	// 클라이언트의 가로, 세로 크기를 반환한다.

	// 다음 프레임으로 진행하는 함수
	void FrameAdvance();
	void WaitForGpuComplete();	// GPU와 동기화을 위한 대기
	void MoveToNextFrame();		// 다음 후면버퍼로 바꾸고 WaitForGpuComplete() 수행

	// 씬 관련 함수들
	void PushScene(const shared_ptr<Scene>& pScene);	// 씬을 추가하고 그 씬으로 전환한다. 호출후 매개변수로 넘겨준 변수를 사용하면 안된다.(move를 통해 이동시키기 때문)
	void PopScene();	// 현재 씬을 제거한다.
	void ChangeScene(const shared_ptr<Scene>& pScene);	// 현재씬을 제거하고 새로운 씬으로 대체한다.
	void ClearScene();	// 모든 씬을 제거한다.
};


