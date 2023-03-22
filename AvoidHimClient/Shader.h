#pragma once

class Texture;
class GameObject;
class Camera;

enum ShaderRenderType {
	PRE_RENDER,			// 정해놓은 멀티 렌더타겟에 그리는 쉐이더
	SWAP_CHAIN_RENDER,	// 후면버퍼에 실제로 그리는 쉐이더
	SHADOW_RENDER,		// 그림자 맵에 그리는 쉐이더
};

class Shader {
protected:
	static weak_ptr<Camera> wpCamera;

	static ComPtr<ID3D12DescriptorHeap>			cbvSrvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE			cbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			cbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorNextHandle;
public:

	static void SetDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> _pCommandList);

	static void CreateCbvSrvDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews);
	// CBV 생성
	static void CreateConstantBufferView();
	// SRV 생성 (단일)
	static void CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);
	// SRV 생성 (다중)
	static void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);

	static void SetCamera(const weak_ptr<Camera>& _wpCamera);
protected:
	ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	ComPtr<ID3D12PipelineState> pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

	// 해당 쉐이더로 그릴 게임 오브젝트들의 포인터 벡터
	vector<weak_ptr<GameObject>> wpGameObjects;
	ShaderRenderType renderType;
public:
	// 생성 관련 함수들
	Shader();
	virtual ~Shader();	

	virtual void PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void AddObject(const weak_ptr<GameObject>& _pGameObject);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return cbvGPUDescriptorStartHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return srvGPUDescriptorStartHandle; };

	const vector<weak_ptr<GameObject>>& GetGameObjects() const;
protected:
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob);

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

class SkinnedShader : public Shader {
private:

public:
	SkinnedShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkinnedShader();
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
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};


class BlendingShader : public Shader {
private:

public:
	BlendingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BlendingShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
};

class EffectShader : public Shader {
private:

public:
	EffectShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~EffectShader();
private:
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

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
	bool InitShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	shared_ptr<Shader> GetShader(const string& _name);
};
