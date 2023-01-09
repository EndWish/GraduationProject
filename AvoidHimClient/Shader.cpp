#include "stdafx.h"
#include "Shader.h"
#include "GameFramework.h"


Shader::Shader() {
}


Shader::~Shader() {

}


void Shader::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pipelineStateDesc.pRootSignature = _pRootSignature.Get();
	pipelineStateDesc.RasterizerState = CreateRasterizerState();
	pipelineStateDesc.BlendState = CreateBlendState();
	pipelineStateDesc.InputLayout = CreateInputLayout();
	pipelineStateDesc.DepthStencilState = CreateDepthStencilState();

	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob) {
	UINT nCompileFlag = 0;

	ComPtr<ID3DBlob> pErrorBlob;
	// 디버그용 플래그	
	nCompileFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	HRESULT hResult = D3DCompileFromFile(_fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, _shaderName.c_str(), _shaderProfile.c_str(), nCompileFlag, 0, &_pBlob, &pErrorBlob);
	char* pErrorString = NULL;
	if (pErrorBlob) {
		pErrorString = (char*)pErrorBlob->GetBufferPointer();
		cout << "바이트 코드 생성 오류 : " << pErrorString << "\n";
	}

	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = _pBlob->GetBufferSize();
	shaderByteCode.pShaderBytecode = _pBlob->GetBufferPointer();

	return shaderByteCode;
}



D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = TRUE;
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

D3D12_BLEND_DESC Shader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
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


void Shader::CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index)
{
	// 셰이더 내 디스크립터 힙에 해당 srv를 생성
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(_Index);

		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
		srvCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetGpuDescriptorHandle(_Index, srvGPUDescriptorNextHandle);
		srvGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
}

void Shader::CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex)
{
	srvCPUDescriptorNextHandle.ptr += (::cbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);
	srvGPUDescriptorNextHandle.ptr += (::cbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);

	int nTextures = _pTexture->GetnTexture();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(i);
		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
		srvCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetGpuDescriptorHandle(i, srvGPUDescriptorNextHandle);

		srvGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = _pTexture->GetnRootParameter();
	for (int i = 0; i < nRootParameters; i++) _pTexture->SetRootParameterIndex(i, _nRootParameterStartIndex + i);
}

void Shader::CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
	for (int i = 0; i < nResources; i++)
	{
		if (ppd3dResources[i])
		{
			auto t = ppd3dResources[i];
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pdxgiSrvFormats[i];
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			// 해당 쉐이더 내에 해당 리소스에 대한 srv 생성
			_pDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
			srvCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
			srvGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
		}
	}
}

void Shader::PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pPipelineState) {
		_pCommandList->SetPipelineState(pPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	// 각 쉐이더가 가지고 있는 디스크립터 힙을 연결한다.
	if (cbvSrvDescriptorHeap) {

		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}
void Shader::PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 스트림 출력
}

void Shader::CreateCbvSrvDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	_pDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&cbvSrvDescriptorHeap));

	cbvCPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cbvGPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	srvCPUDescriptorStartHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);
	srvGPUDescriptorStartHandle.ptr = cbvGPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);

	srvCPUDescriptorNextHandle = srvCPUDescriptorStartHandle;
	srvGPUDescriptorNextHandle = srvGPUDescriptorStartHandle;
}


//////////////////// Basic Shader
BasicShader::BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 50);

	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "BasicShader 생성 성공\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}

BasicShader::~BasicShader() {

}





D3D12_RASTERIZER_DESC BasicShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC BasicShader::CreateInputLayout() {
	inputElementDescs.assign(3, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}


/////////////////////////    Shader Manager   ////////////////////////////////

bool ShaderManager::InitShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature)
{
	shared_ptr<Shader> basicShader = make_shared<BasicShader>(_pDevice, _pRootSignature);
	if (basicShader) storage["BasicShader"] = basicShader;
	else return false;
	// 이후에 계속 추가


	return true;
}

shared_ptr<Shader> ShaderManager::GetShader(const string& _name)
{
	if (storage[_name])
		return storage[_name];

	else {
		cout << "해당 쉐이더가 존재하지 않음\n";
		return nullptr;
	}
	
}

