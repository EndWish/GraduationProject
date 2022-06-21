#pragma once
class GameFramework {
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	//Ŭ���̾�Ʈ �ػ�
	int m_windowClientWidth;
	int m_windowClientHeight;

	IDXGIFactory4* m_pDxgiFactory;	// DXGI ���丮 �������̽��� ���� �������̴�.
	IDXGISwapChain3* m_pDxgiSwapChain;	// ���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���� ���
	ID3D12Device* m_pD3dDevice;	///Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�
	bool m_msaa4xEnable;
	UINT m_msaa4xQualityLevels;

	static const UINT m_nSwapChainBuffers = 2;	//���� ü���� �ĸ� ������ �����̴�.
	UINT m_swapChainBufferCurrentIndex; //���� ���� ü���� �ĸ� ���� �ε����̴�.

	//���� Ÿ�� ����, ������ �� �������̽� ������, 
	ID3D12Resource* m_ppD3dRenderTargetBuffers[m_nSwapChainBuffers];	//����Ÿ�ٹ���(�ĸ����) �����͵�
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;	//����Ÿ�� ������ �� ������, ������ ���� �����ڵ��� ��� �޸��̴�. �����ڴ� ���ҽ����� �޸𸮸� �������ִ� �Ű�ü https://ssinyoung.tistory.com/37 ����
	UINT m_rtvDescriptorIncrementSize;	//���� Ÿ�� ������ ������ ũ���̴�.
	
	//����-���ٽ� ����, ������ �� �������̽� ������, ����-���ٽ� ������ ������ ũ���̴�.
	ID3D12Resource* m_pD3dDepthStencilBuffer;	// ����-���ٽ� ���۴� ����Ÿ�ٹ��ۿ� �ٸ��� 1���� ������ �ȴ�.
	ID3D12DescriptorHeap* m_pD3dDsvDescriptorHeap;
	UINT m_dsvDescriptorIncrementSize;

	//��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�
	ID3D12CommandQueue* m_pD3dCommandQueue;
	ID3D12CommandAllocator* m_pD3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pD3dCommandList;
	
	//�潺 �������̽� ������, �̺�Ʈ �ڵ�, �潺�� �� �̴�. 
	ID3D12Fence* m_pD3dFence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[m_nSwapChainBuffers];
	

public:
	// ������ �� �Ҹ���
	GameFramework();
	~GameFramework();

};


