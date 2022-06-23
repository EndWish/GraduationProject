#pragma once
class GameFramework {
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	//Ŭ���̾�Ʈ �ػ�
	int m_windowClientWidth;
	int m_windowClientHeight;

	// DXGI(DirectX Graphics Infrastructure) : DirectX �׷����� ��Ÿ�ӿ� �������� �������� �۾��� ��������, �׷����� ���� �⺻���̰� �������� �����̿�ũ�� �����Ѵ�.
	// ComPtr�� Com��ü�� ���� ����Ʈ ������ �̴�.
	ComPtr<IDXGIFactory4> m_pDxgiFactory;	// DXGI ���丮 �������̽��� ���� �������̴�. (��͸� �����ϴ� ����, )
	ComPtr<IDXGISwapChain3> m_pDxgiSwapChain;	// ���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���� ���
	ComPtr<ID3D12Device> m_pD3dDevice;	///Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�
	bool m_msaa4xEnable;
	UINT m_msaa4xQualityLevels;

	static const UINT m_nSwapChainBuffers = 2;	//���� ü���� �ĸ� ������ �����̴�.
	UINT m_swapChainBufferCurrentIndex; //���� ���� ü���� �ĸ� ���� �ε����̴�.

	//���� Ÿ�� ����, ������ �� �������̽� ������, 
	ComPtr<ID3D12Resource> m_ppD3dRenderTargetBuffers[m_nSwapChainBuffers];	//����Ÿ�ٹ���(�ĸ����) �����͵�
	ComPtr<ID3D12DescriptorHeap> m_pD3dRtvDescriptorHeap;	//����Ÿ�� ������ �� ������, ������ ���� �����ڵ��� ��� �޸��̴�. �����ڴ� ���ҽ����� �޸𸮸� �������ִ� �Ű�ü https://ssinyoung.tistory.com/37 ����
	UINT m_rtvDescriptorIncrementSize;	//���� Ÿ�� ������ ������ ũ���̴�.
	
	//����-���ٽ� ����, ������ �� �������̽� ������, ����-���ٽ� ������ ������ ũ���̴�.
	ComPtr<ID3D12Resource> m_pD3dDepthStencilBuffer;	// ����-���ٽ� ���۴� ����Ÿ�ٹ��ۿ� �ٸ��� 1���� ������ �ȴ�.
	ComPtr<ID3D12DescriptorHeap> m_pD3dDsvDescriptorHeap;
	UINT m_dsvDescriptorIncrementSize;

	//��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�
	ComPtr<ID3D12CommandQueue> m_pD3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pD3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pD3dCommandList;
	
	//�潺 �������̽� ������, �̺�Ʈ �ڵ�, �潺�� �� �̴�. 
	ComPtr<ID3D12Fence> m_pD3dFence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[m_nSwapChainBuffers];
	

public:
	// ������ �� �Ҹ���
	GameFramework();
	~GameFramework();

	// ������(�ʱ�ȭ) �Լ�
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);	// �������ϴ� �Լ�

	// ������(�ʱ�ȭ) �Լ��� �κе�
	void CreateDirect3dDevice();	// dxgiFactory�� �����ϰ�, �װ��� �̿��Ͽ� �����(�׷��� ī��)�� ���켭 ������ �׷���ī��� ����̽��� �����Ѵ�.
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();
	void CreateRenderTargetViews();		// GPU���� ���� �� �ֵ��� ����Ÿ�� �並 ���� ( CreateSwapChain()�� ȭ�� ũ�� ��ȯ�Ҷ� ���ȴ�.  )
	//void CreateDepthStencilView();


};


