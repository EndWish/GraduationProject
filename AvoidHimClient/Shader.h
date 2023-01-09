#pragma once

class Texture;

class Shader {
protected:
	ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	ComPtr<ID3D12PipelineState> pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;


	ComPtr<ID3D12DescriptorHeap>		cbvSrvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			cbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			cbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorNextHandle;
public:
	// 积己 包访 窃荐甸
	Shader();
	virtual ~Shader();
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob);

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	void CreateCbvSrvDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() = 0;
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() = 0;
	virtual void PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	// CBV 积己
	void CreateConstantBufferView();
	// SRV 积己 (窜老)
	void CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);
	// SRV 积己 (促吝)
	void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex);

	void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);

	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);


	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return cbvGPUDescriptorStartHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return srvGPUDescriptorStartHandle; };

};

class BasicShader : public Shader {
private:

public:
	BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BasicShader();

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
