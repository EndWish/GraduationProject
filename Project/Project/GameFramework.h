#pragma once
#include "Timer.h"
#include "Scene.h"

class GameFramework {
// ���� ����
private:
	static unique_ptr<GameFramework> spInstance;	// ���� �����ӿ�ũ 

public:
	static void Create(HINSTANCE _hInstance, HWND _hMainWnd);
	static void Destroy();

	// �����ӿ�ũ �ν��Ͻ��� �������� ���� �ϱ� ���� �Լ�
	static GameFramework& Instance();
private:
	HINSTANCE instanceHandle;
	HWND windowHandle;
	int clientWidth;
	int clientHeight;


	ComPtr<IDXGIFactory4> pDxgiFactory;										// DXGI ���丮 �������̽��� ���� ������
	ComPtr<IDXGISwapChain3> pDxgiSwapChain;									// ����ü�� �������̽�
	ComPtr<ID3D12Device> pDevice; 

	bool msaa4xEnable;
	UINT msaa4xLevel;

	static const UINT nSwapChainBuffer = 2;									// ���� ü�� ���� ����
	UINT swapChainBufferCurrentIndex;										// ���� �׸� ����ü���� �ĸ���� �ε��� 
	float t = 0;

	// ����Ÿ��, ����-���ٽ� ����
	array<ComPtr<ID3D12Resource>, nSwapChainBuffer> pRenderTargetBuffers;	// ���� Ÿ�� ����(�ĸ�) �����͸� ��� �迭
	ComPtr<ID3D12DescriptorHeap> pRtvDescriptorHeap;						// ����Ÿ�ٹ����� ������ ��. 
	UINT rtvDescriptorIncrementSize;										// ����Ÿ�� �������� ũ��

	ComPtr<ID3D12Resource> pDepthStencilBuffer;								// ����-���ٽ� ���� �����͸� ��� �迭
	ComPtr<ID3D12DescriptorHeap> pDsvDescriptorHeap;						// ����-���ٽ� ������ ������ ��. 
	UINT dsvDescriptorIncrementSize;										// ����-���ٽ� ������ �������� ũ��


	// ��� ť, ��� �Ҵ���, ��� ����Ʈ�� �������̽� ������
	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;

	// �潺
	ComPtr<ID3D12Fence> pFence;
	HANDLE fenceEvent;
	array<UINT64, nSwapChainBuffer> fenceValues;
	
	// ��Ʈ �ñ״�ó
	ComPtr<ID3D12RootSignature> pRootSignature;

	// ���� Ÿ�̸�
	Timer gameTimer;
	
	// ��
	stack<shared_ptr<Scene>> pScenes;

// ����, �Ҹ���
private:
	GameFramework();
public:
	~GameFramework();

// ������ �ʱ�ȭ�� �Լ�
	void CreateDirect3DDevice();		// dxgiFactory�� �����ϰ�, �װ��� �̿��Ͽ� ����̽��� ����
	void CreateCommandQueueList();		// Ŀ�ǵ� ť ����Ʈ ����
	void CreateRtvAndDsvDescriptorHeaps();	// ������ �� ����
	void CreateSwapChain();				// ����ü�� ����
	void CreateRenderTargetViews();		// ����Ÿ�ٺ� ���� �� ������ ���� ����
	void CreateDepthStencilView();
	void CreateGraphicsRootSignature();	// ��Ʈ �ñ״��� ����

// get set �Լ�
public:
	void FrameAdvance();
	void WaitForGpuComplete();			// GPU�� ����ȭ�ϱ� ���� ���
	void MoveToNextFrame();				// ���� �ĸ���۷� ������ WaitForGpuComplete() ����

	void ChangeSwapChainState();
	void ProcessInput();

	void PushScene(const shared_ptr<Scene>& _pScene);
	void PopScene();
	void ChangeScene(const shared_ptr<Scene>& _pScene);
	void ClearScene();
};
