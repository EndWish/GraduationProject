#pragma once

#include "GameTimer.h"
#include "Scenes.h"

class GameFramework {
private:	// ���� ������
	static unique_ptr<GameFramework> s_pInstance;	// �̱��� ����, https://boycoding.tistory.com/109

public:		// ���� �Լ���
	// ������(�ʱ�ȭ) �Լ� �� �Ҹ�� �Լ�
	static bool Create(HINSTANCE hInstance, HWND hMainWnd);
	static void Destroy();

	// ���� ������ ������ �� �ִ� �Լ�
	static GameFramework& Instance();

private:	// ��� ������
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	//------------------------------------����̽����� ����-------------------------------------
	//Ŭ���̾�Ʈ �ػ�
	int m_windowClientWidth;
	int m_windowClientHeight;

	// DXGI(DirectX Graphics Infrastructure) : DirectX �׷����� ��Ÿ�ӿ� �������� �������� �۾��� ��������, �׷����� ���� �⺻���̰� �������� �����̿�ũ�� �����Ѵ�.
	// ComPtr�� Com��ü�� ���� ����Ʈ ������ �̴�.
	ComPtr<IDXGIFactory4> m_pDxgiFactory;	// DXGI ���丮 �������̽��� ���� �������̴�. (��͸� �����ϴ� ����, )
	ComPtr<IDXGISwapChain3> m_pDxgiSwapChain;	// ���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���� ���
	ComPtr<ID3D12Device> m_pDevice;	///Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�
	bool m_msaa4xEnable;
	UINT m_msaa4xQualityLevels;

	static const UINT m_nSwapChainBuffers = 2;	//���� ü���� �ĸ� ������ �����̴�.
	UINT m_swapChainBufferCurrentIndex; //���� ���� ü���� �ĸ� ���� �ε����̴�.

	//���� Ÿ�� ����, ������ �� �������̽� ������, 
	ComPtr<ID3D12Resource> m_ppRenderTargetBuffers[m_nSwapChainBuffers];	//����Ÿ�ٹ���(�ĸ����) �����͵�
	ComPtr<ID3D12DescriptorHeap> m_pRtvDescriptorHeap;	//����Ÿ�� ������ �� ������, ������ ���� �����ڵ��� ��� �޸��̴�. �����ڴ� ���ҽ����� �޸𸮸� �������ִ� �Ű�ü https://ssinyoung.tistory.com/37 ����
	UINT m_rtvDescriptorIncrementSize;	//���� Ÿ�� ������ ������ ũ���̴�.
	
	//����-���ٽ� ����, ������ �� �������̽� ������, ����-���ٽ� ������ ������ ũ���̴�.
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;	// ����-���ٽ� ���۴� ����Ÿ�ٹ��ۿ� �ٸ��� 1���� ������ �ȴ�.
	ComPtr<ID3D12DescriptorHeap> m_pDsvDescriptorHeap;
	UINT m_dsvDescriptorIncrementSize;

	//��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	
	//�潺 �������̽� ������, �̺�Ʈ �ڵ�, �潺�� �� �̴�. 
	ComPtr<ID3D12Fence> m_pFence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[m_nSwapChainBuffers];

	//��Ʈ �ñ״�ó ����
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	
	//------------------------------------���Ӱ��� ����-------------------------------------
	//
	GameTimer m_gameTimer;
	stack<shared_ptr<Scene>> m_pScenes;	// ������ �����Ѵ�. top�� �ִ� ���� ���� ����� ���̴�.

public:	// �������� ��� �Լ���
	~GameFramework();	// �Ҹ���
private:
	GameFramework();	// ������

	// ������(�ʱ�ȭ) �Լ��� �κе�
	void CreateDirect3dDevice();	// dxgiFactory�� �����ϰ�, �װ��� �̿��Ͽ� �����(�׷��� ī��)�� ���켭 ������ �׷���ī��� ����̽��� �����Ѵ�.
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();
	void CreateRenderTargetViews();		// GPU���� ���� �� �ֵ��� ����Ÿ�� �並 ���� ( CreateSwapChain()�� ȭ�� ũ�� ��ȯ�Ҷ� ���ȴ�.  )
	void CreateDepthStencilView();	// ����-���ٽ� ����(���ҽ�)�� ����� ����-���ٽ� �並 ������ ���� ����
	void CreateGraphicsRootSignature();	// ��Ʈ �ñ״�ó�� �����Ѵ�.

public:		// ��� �Լ���
	// Get Set �Լ�
	Scene& GetCurrentSceneRef();
	pair<int, int> GetWindowClientSize() const;	// Ŭ���̾�Ʈ�� ����, ���� ũ�⸦ ��ȯ�Ѵ�.

	// ���� ���������� �����ϴ� �Լ�
	void FrameAdvance();
	void WaitForGpuComplete();	// GPU�� ����ȭ�� ���� ���
	void MoveToNextFrame();		// ���� �ĸ���۷� �ٲٰ� WaitForGpuComplete() ����

	// �� ���� �Լ���
	void PushScene(const shared_ptr<Scene>& pScene);	// ���� �߰��ϰ� �� ������ ��ȯ�Ѵ�. ȣ���� �Ű������� �Ѱ��� ������ ����ϸ� �ȵȴ�.(move�� ���� �̵���Ű�� ����)
	void PopScene();	// ���� ���� �����Ѵ�.
	void ChangeScene(const shared_ptr<Scene>& pScene);	// ������� �����ϰ� ���ο� ������ ��ü�Ѵ�.
	void ClearScene();	// ��� ���� �����Ѵ�.
};


