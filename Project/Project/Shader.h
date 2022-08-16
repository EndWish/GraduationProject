#pragma once
class Shader {
protected:
	ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	ComPtr<ID3D12PipelineState> pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	vector<D3D12_INPUT_ELEMENT_DESC> pInputElementDescs;

public:
	// 생성 관련 함수들
	Shader();
	~Shader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob);
	D3D12_RASTERIZER_DESC CreateRasterizerState();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	D3D12_BLEND_DESC CreateBlendState();
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	//
	void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};

