#pragma once

class Texture;
class GameObject;
class Camera;

enum class PARTICLE_TYPE {
	waterDispenserUse,
	healItemUse,
	throwAttack,
	LeverUse,
	KeyUse,
};

struct VS_ParticleMappedFormat {
	XMFLOAT3 position;
	XMFLOAT3 velocity;
	XMFLOAT2 boardSize;
	float lifetime;
	UINT type;
};

struct ParticleResource {
	static const UINT nMaxParticle = 10000;
	UINT nDefaultStreamInputParticle = 0;	// defaultStreamInputBuffer�� ��ƼŬ�� ����
	UINT nUploadStreamInputParticle = 0;	// uploadStreamInputBuffer�� ��ƼŬ�� ����
	UINT nDefaultStreamOutputParticle = 0;
	ComPtr<ID3D12Resource> uploadStreamInputBuffer, defaultStreamInputBuffer, defaultDrawBuffer;	// ���� ��ƼŬ �߰��� ���� upload_heap����, ��Ʈ������� �Է��� ����� ����, ��Ʈ�� ����� ����� �� ���� 
	shared_ptr<VS_ParticleMappedFormat[nMaxParticle]> mappedUploadStreamInputBuffer;	// upload���ۿ� ���� �������� �������� ������
	D3D12_VERTEX_BUFFER_VIEW uploadStreamInputBufferView, defaultStreamInputBufferView;	// ��Ʈ�� ����� �Է����� �� ���ҽ��� ���� ��
	D3D12_STREAM_OUTPUT_BUFFER_VIEW defaultStreamOutputBufferView;	// ��Ʈ���� ����� �� ���ҽ��� ���� ��
	D3D12_VERTEX_BUFFER_VIEW defaultDrawBufferView;	// ��µ� ���ҽ����� �׸��� ���� ����� ��
	ComPtr<ID3D12Resource> defaultBufferFilledSize, uploadBufferFilledSize, readBackBufferFilledSize;	// ���� SO���� write�� ũ�Ⱑ �Էµ� ����, default���ۿ� 0�� �������� ����, defualt�� ���� ���� �о���� ���� ����
	shared_ptr<UINT> mappedReadBackBufferFilledSize;

	// �ؽ�ó ���� ����
	shared_ptr<Texture> texture;
	ComPtr<ID3D12DescriptorHeap> textureDescriptorHeap;
	//shared_ptr<TextureBundle> textures;


	ParticleResource() {}

	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};

enum ShaderRenderType {
	PRE_RENDER,			// ���س��� ��Ƽ ����Ÿ�ٿ� �׸��� ���̴�
	SWAP_CHAIN_RENDER,	// �ĸ���ۿ� ������ �׸��� ���̴�
	SHADOW_RENDER,		// �׸��� �ʿ� �׸��� ���̴�
};

class Shader {
protected:
	static weak_ptr<Camera> wpCamera;

	static ComPtr<ID3D12DescriptorHeap>			cbvSrvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE			cbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			cbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			uavCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			uavGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorNextHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			uavCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			uavGPUDescriptorNextHandle;

	static ComPtr<ID3D12DescriptorHeap>			computeDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE			cbvComputeCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			cbvComputeGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE			srvComputeCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvComputeGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			uavComputeCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			uavComputeGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			srvComputeCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvComputeGPUDescriptorNextHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			uavComputeCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			uavComputeGPUDescriptorNextHandle;
	static ParticleResource particleResource;
public:

	static void SetDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> _pCommandList);

	static void CreateCbvSrvUavDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);
	
	static void SetComputeDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> _pCommandList);

	static void CreateComputeDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

	// CBV ����
	static void CreateConstantBufferView();
	// SRV ���� (����)
	static void CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);
	// SRV ���� (����)
	static void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);

	// UAV ����
	static void CreateUnorderedAccessView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);

	// SRV ���� (����)
	static void CreateComputeShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);
	// SRV ���� (����)
	static void CreateComputeShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex);
	// UAV ����
	static void CreateComputeUnorderedAccessView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);


	//  StreamOutput�� ���õ� �Լ���
	static void AddParticle(const VS_ParticleMappedFormat& _particle);
	static void RenderParticle(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	static void SetCamera(const weak_ptr<Camera>& _wpCamera);
	
	static void UpdateShadersObject();

	static void InitParticleResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) { particleResource.Init(_pDevice, _pCommandList); }
protected:
	ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	ComPtr<ID3D12PipelineState> pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

	// �ش� ���̴��� �׸� ���� ������Ʈ���� ������ ����
	vector<weak_ptr<GameObject>> wpGameObjects;
	ShaderRenderType renderType;
public:
	// ���� ���� �Լ���
	Shader();
	virtual ~Shader();	

	virtual void PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void AddObject(const weak_ptr<GameObject>& _pGameObject);

	virtual void ClearObject();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return cbvGPUDescriptorStartHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return srvGPUDescriptorStartHandle; };

	vector<weak_ptr<GameObject>>& GetGameObjects();

	void UpdateShaderObject();
protected:
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob);
	D3D12_SHADER_BYTECODE LoadShaderFromFile(const wstring& _fileName, ComPtr<ID3DBlob>& _pBlob);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() = 0;
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() = 0;

};

class BasicShader : public Shader {
private:

public:
	BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BasicShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class BasicShadowShader : public Shader {
private:

public:
	BasicShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BasicShadowShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class SkinnedShader : public Shader {
private:

public:
	SkinnedShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkinnedShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class SkinnedShadowShader : public Shader {
private:

public:
	SkinnedShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkinnedShadowShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class SkinnedTransparentShader : public Shader {
private:

public:
	SkinnedTransparentShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkinnedTransparentShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
};

class SkinnedLobbyShader : public Shader {
private:

public:
	SkinnedLobbyShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkinnedLobbyShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class UIShader : public Shader {

public:
	UIShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~UIShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
};

class BoundingMeshShader : public Shader {
public:
	BoundingMeshShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BoundingMeshShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class InstancingShader : public Shader {
private:

public:
	InstancingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~InstancingShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class InstancingShadowShader : public Shader {
private:

public:
	InstancingShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~InstancingShadowShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};



class BlendingShader : public Shader {
private:

public:
	BlendingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BlendingShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();


};

class EffectShader : public Shader {
private:

public:
	EffectShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~EffectShader();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();


};

class SkyBoxShader : public Shader {
private:

public:
	SkyBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkyBoxShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;

};

class LightingShader : public Shader {
private:

public:
	LightingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~LightingShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

/////////////////////////    Shader Manager   ////////////////////////////////

class ShaderManager {
private:
	unordered_map<string, shared_ptr<Shader>> storage;

public:
	bool InitShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature, const ComPtr<ID3D12RootSignature>& _pComputeRootSiganture);
	shared_ptr<Shader> GetShader(const string& _name);
};

///////////////////////////////////////////////////////////////////////////////

class ParticleStreamOutShader : public Shader {
protected:
	ComPtr<ID3DBlob> pGSBlob;
public:
	ParticleStreamOutShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~ParticleStreamOutShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) { cout << "������ �ʴ´�.\n"; };	// Render ���������Լ�?
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool isUploadInput);	// Render ���������Լ�?
};



class ParticleDrawShader : public Shader {
protected:
	ComPtr<ID3DBlob> pGSBlob;
public:
	ParticleDrawShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~ParticleDrawShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class BlurComputeShader : public Shader {

protected:
	XMUINT3 numThreads;
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc;
	ComPtr<ID3DBlob> pCSBlob;
public:
	BlurComputeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BlurComputeShader();

	virtual void Dispatch(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
};

class PostShader : public Shader {
private:

public:
	
public:
	PostShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~PostShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};