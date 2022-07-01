#pragma once

class Shader {
public:	// 정적 변수▼
	static Shader instance;

protected:	// 멤버 변수▼
	ComPtr<ID3D10Blob> m_pVertexShaderBolb;
	ComPtr<ID3D10Blob> m_pPixelShaderBolb;

	ComPtr<ID3D12PipelineState> m_pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_pipelineStateDesc;

public:		// 생성관련 멤버 함수▼
	Shader();
	virtual ~Shader();

	virtual void CreateShader(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12RootSignature>& pRootSignature);

protected:
	// CreateShader에 포함되는 생성 함수들
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();	// 정점쉐이더 생성, (CreateShader에 포함됨)
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();	// 픽셀쉐이더 생성, (CreateShader에 포함됨)
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();	// 레스터 라이저 생성, (CreateShader에 포함됨)
	virtual D3D12_BLEND_DESC CreateBlendState();	// 색상을 어떻게 섞는지에 관한 상태
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();	// 깊이-스텐실 설정 생성

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);	// 컴파일하는 함수

public:		// 멤버 함수▼
	virtual void OnPrepareRender(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);	// 파이프라인 상태 세팅


};

