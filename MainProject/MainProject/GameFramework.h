#pragma once
class GameFramework {
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	//클라이언트 해상도
	int m_windowClientWidth;
	int m_windowClientHeight;

	// DXGI(DirectX Graphics Infrastructure) : DirectX 그래픽을 런타임에 독립적인 저수준의 작업을 관리해줌, 그래픽을 위한 기본적이고 공통적인 프레이워크를 제공한다.
	// ComPtr는 Com객체에 대한 스마트 포인터 이다.
	ComPtr<IDXGIFactory4> m_pDxgiFactory;	// DXGI 팩토리 인터페이스에 대한 포인터이다. (어뎁터를 열거하는 역할, )
	ComPtr<IDXGISwapChain3> m_pDxgiSwapChain;	// 스왑 체인 인터페이스에 대한 포인터이다. 주로 디스플레이를 제어하기 위해 사용
	ComPtr<ID3D12Device> m_pD3dDevice;	///Direct3D 디바이스 인터페이스에 대한 포인터이다. 주로 리소스를 생성하기 위하여 필요하다.

	//MSAA 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다
	bool m_msaa4xEnable;
	UINT m_msaa4xQualityLevels;

	static const UINT m_nSwapChainBuffers = 2;	//스왑 체인의 후면 버퍼의 개수이다.
	UINT m_swapChainBufferCurrentIndex; //현재 스왑 체인의 후면 버퍼 인덱스이다.

	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 
	ComPtr<ID3D12Resource> m_ppD3dRenderTargetBuffers[m_nSwapChainBuffers];	//렌더타겟버퍼(후면버퍼) 포인터들
	ComPtr<ID3D12DescriptorHeap> m_pD3dRtvDescriptorHeap;	//렌더타켓 서술자 힙 포인터, 서술자 힙은 서술자들을 담는 메모리이다. 서술자는 리소스가상 메모리를 관리해주는 매개체 https://ssinyoung.tistory.com/37 참고
	UINT m_rtvDescriptorIncrementSize;	//렌더 타겟 서술자 원소의 크기이다.
	
	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기이다.
	ComPtr<ID3D12Resource> m_pD3dDepthStencilBuffer;	// 깊이-스텐실 버퍼는 렌더타겟버퍼와 다르게 1개만 있으면 된다.
	ComPtr<ID3D12DescriptorHeap> m_pD3dDsvDescriptorHeap;
	UINT m_dsvDescriptorIncrementSize;

	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다
	ComPtr<ID3D12CommandQueue> m_pD3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pD3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pD3dCommandList;
	
	//펜스 인터페이스 포인터, 이벤트 핸들, 펜스의 값 이다. 
	ComPtr<ID3D12Fence> m_pD3dFence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[m_nSwapChainBuffers];
	

public:
	// 생성자 및 소멸자
	GameFramework();
	~GameFramework();

	// 생성시(초기화) 함수
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);	// ㅇㅇㅇ하는 함수

	// 생성시(초기화) 함수의 부분들
	void CreateDirect3dDevice();	// dxgiFactory를 생성하고, 그것을 이용하여 어댑터(그래픽 카드)를 살펴서 적절한 그래픽카드로 디바이스를 생성한다.
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();
	void CreateRenderTargetViews();		// GPU에서 읽을 수 있도록 렌더타겟 뷰를 생성 ( CreateSwapChain()과 화면 크기 전환할때 사용된다.  )
	//void CreateDepthStencilView();


};


