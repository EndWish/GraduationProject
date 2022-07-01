#pragma once

class Shader {
public:	// ���� ������
	static Shader instance;

protected:	// ��� ������
	ComPtr<ID3D10Blob> m_pVertexShaderBolb;
	ComPtr<ID3D10Blob> m_pPixelShaderBolb;

	ComPtr<ID3D12PipelineState> m_pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_pipelineStateDesc;

public:		// �������� ��� �Լ���
	Shader();
	virtual ~Shader();

	virtual void CreateShader(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12RootSignature>& pRootSignature);

protected:
	// CreateShader�� ���ԵǴ� ���� �Լ���
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();	// �������̴� ����, (CreateShader�� ���Ե�)
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();	// �ȼ����̴� ����, (CreateShader�� ���Ե�)
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();	// ������ ������ ����, (CreateShader�� ���Ե�)
	virtual D3D12_BLEND_DESC CreateBlendState();	// ������ ��� �������� ���� ����
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();	// ����-���ٽ� ���� ����

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);	// �������ϴ� �Լ�

public:		// ��� �Լ���
	virtual void OnPrepareRender(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);	// ���������� ���� ����


};

