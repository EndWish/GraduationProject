#include "stdafx.h"
#include "Shader.h"

/// 정적 변수▼
Shader Shader::instance = Shader();

/// 멤버 변수▼
// 생성자 및 소멸자
Shader::Shader() 
{
	m_pipelineStateDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC();
}
Shader::~Shader() {

}

// 쉐이더 생성
void Shader::CreateShader(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12RootSignature>& pRootSignature) {
	::ZeroMemory(&m_pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_pipelineStateDesc.pRootSignature = pRootSignature.Get();
	m_pipelineStateDesc.VS = CreateVertexShader();
	m_pipelineStateDesc.PS = CreatePixelShader();
	m_pipelineStateDesc.InputLayout = CreateInputLayout();
	m_pipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_pipelineStateDesc.BlendState = CreateBlendState();
	m_pipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_pipelineStateDesc.SampleMask = UINT_MAX;
	m_pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_pipelineStateDesc.NumRenderTargets = 1;
	m_pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_pipelineStateDesc.SampleDesc.Count = 1;
	m_pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pDevice->CreateGraphicsPipelineState(&m_pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pPipelineState);
}

// 정점,픽셀 쉐이더를 파일로 부터 컴파일해서 가져오기
D3D12_SHADER_BYTECODE Shader::CreateVertexShader() 
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSPrototype", "vs_5_1", &m_pVertexShaderBolb);
}
D3D12_SHADER_BYTECODE Shader::CreatePixelShader() 
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSPrototype", "ps_5_1", &m_pPixelShaderBolb);
}
D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout() {
	// Shader는 Position만 받아 그리는 쉐이더
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* pInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = pInputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}
D3D12_RASTERIZER_DESC Shader::CreateRasterizerState() 
{
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	// 채워서 그리기
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;		// 뒷면 컬링
	rasterizerDesc.FrontCounterClockwise = FALSE;		// 반시계를 앞면으로 = False
	rasterizerDesc.DepthBias = 0;						// 깊이 바이오스(아마 그림자할때 사용할 듯)
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}
D3D12_BLEND_DESC Shader::CreateBlendState() 
{
	D3D12_BLEND_DESC blendDesc;
	::ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}
D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState() 
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	::ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;									// 깊이 사용
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

// 컴파일 하는 함수
D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob) {
	UINT nCompileFlags = 0;

	ComPtr<ID3DBlob> pErrorBlob;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pErrorBlob);
#ifdef DEBUG
	if (pErrorBlob) {
		char* pErrorString = (char*)pErrorBlob->GetBufferPointer();
		cout << "Shader::CompileShaderFromFile() : " << pErrorString << "\n";
		ofstream out{ "에러 내용.txt" };
		out << "Shader::CompileShaderFromFile() : " << pErrorString << "\n";
	}
#endif // DEBUG

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return d3dShaderByteCode;
}

// 파이프라인 세팅
void Shader::OnPrepareRender(const ComPtr<ID3D12GraphicsCommandList>& pCommandList) {
	// 이미 같은 상태로 세팅이 되어 있다면 
	pCommandList->SetPipelineState(m_pPipelineState.Get());
}

