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

	ComPtr<IDXGIFactory4> pDxgiFactory;										// DXGI ���丮 �������̽��� ���� ������
	ComPtr<IDXGISwapChain3> pDxgiSwapChain;									// ����ü�� �������̽�
	ComPtr<ID3D12Device> pDevice;

	bool msaa4xEnable;
	UINT msaa4xLevel;

	static const UINT nSwapChainBuffer = 2;									// ���� ü�� ���� ����
	UINT swapChainBufferCurrentIndex;										// ���� �׸� ����ü���� �ĸ���� �ε��� 

	// ����Ÿ��, ����-���ٽ� ����
	array<ComPtr<ID3D12Resource>, nSwapChainBuffer> pRenderTargetBuffers;	// ���� Ÿ�� ����(�ĸ�) �����͸� ��� �迭
	array<D3D12_CPU_DESCRIPTOR_HANDLE, nSwapChainBuffer> rtvCPUDescriptorHandles;
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

	// ��ǻƮ ���̴��� ���� ��Ʈ �ñ״�ó
	ComPtr<ID3D12RootSignature> pComputeRootSignature;

	// ���� Ÿ�̸�
	Timer gameTimer;

	// ���� �����ӿ����� ���콺 Ŀ�� ��ġ�� �����ϴ� ����
	POINT oldCursorPos;

	HitBoxMesh hitBoxMesh;
	// ���� ���� ���콺�� ������ �ִ����� �Ǵ�
	bool isClick;

	///// �Ʒ��� Instance()���� �ٸ� �Լ��� ���� ������ ������
	
	int clientWidth;
	int clientHeight;
	
	// ���׸��� �Ŵ���
	TextureManager textureManager;
	
	// ���� ������Ʈ �Ŵ���
	GameObjectManager gameObjectManager;
	
	ShaderManager shaderManager;

	// ���� �Ŵ���
	SoundManager pSoundManager;

	// �ִϸ��̼� �Ŵ��� ? 
	// ��
	stack<shared_ptr<Scene>> pScenes;

	// G Buffer
	shared_ptr<Texture> pGBuffer;
	// G Buffer�� ����Ÿ�ٺ� �ּ�
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> GBufferCPUDescriptorHandles;
	
	// 
	vector < shared_ptr<Texture>>pBakedShadowMaps;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> bakedShadowMapCPUDescriptorHandles;


	// �� ������ �׷����� ������ ��
	shared_ptr<Texture> pDynamicShadowMap;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> dynamicShadowMapCPUDescriptorHandles;
	
	// ������ �� = dynamic + baked
	shared_ptr<Texture> pShadowMap;

	// �� ó���� ��ģ ȭ�� ����
	shared_ptr<Texture> pPostBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE postBufferCPUDescriptorHandle;

	// ��ǻƮ ���̴��� ��� ����� ����� ����
	shared_ptr<Texture> pComputeBuffer;

	// ������ ��
	shared_ptr<Texture> pWireFrameMap;
	D3D12_CPU_DESCRIPTOR_HANDLE wireFrameMapCPUDescriptorHandle;

	// ��ǻƮ ���̴��� ��� ����� ������ ����
	shared_ptr<Texture> pDestBuffer;

	// ���̴� ����
	ComPtr<ID3D12Resource> pcbFrameworkInfo;
	shared_ptr<CB_FRAMEWORK_INFO> pcbMappedFrameworkInfo;


	// ����, �Ҹ���
private:
	GameFramework();
public:
	~GameFramework();

	// ������ �ʱ�ȭ�� �Լ�
protected:
	void CreateDirect3DDevice();		// dxgiFactory�� �����ϰ�, �װ��� �̿��Ͽ� ����̽��� ����
	void CreateCommandQueueList();		// Ŀ�ǵ� ť ����Ʈ ����
	void CreateRtvAndDsvDescriptorHeaps();	// ������ �� ����
	void CreateSwapChain();				// ����ü�� ����
	void CreateRenderTargetViews();		// ����Ÿ�ٺ� ���� �� ������ ���� ����
	void CreateDepthStencilView();
	void CreateGraphicsRootSignature();	// ��Ʈ �ñ״��� ����
	void CreateComputeRootSignature();	// ��Ʈ �ñ״��� ����
	
	// ���̴� ����
	bool InitShader();
	void InitBuffer();
	
	// Frame Advance

	void BakeShadowMap();
	void RenderDynamicShadowMap();
	void MergeShadowMap();
	void RenderGBuffer();
	void RenderWireFrame();
	void LightingAndComputeBlur();
	// get set �Լ�


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
	void WaitForGpuComplete();			// GPU�� ����ȭ�ϱ� ���� ���
	void MoveToNextFrame();				// ���� �ĸ���۷� ������ WaitForGpuComplete() ����


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
