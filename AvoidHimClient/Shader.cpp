#include "stdafx.h"
#include "Shader.h"
#include "GameFramework.h"

ComPtr<ID3D12DescriptorHeap> Shader::cbvSrvDescriptorHeap;
ComPtr<ID3D12DescriptorHeap> Shader::computeDescriptorHeap;

D3D12_CPU_DESCRIPTOR_HANDLE Shader::srvCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::srvGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::uavCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::uavGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::srvCPUDescriptorNextHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::srvGPUDescriptorNextHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::uavCPUDescriptorNextHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::uavGPUDescriptorNextHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::cbvCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::cbvGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::srvComputeCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::srvComputeGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::uavComputeCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::uavComputeGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::srvComputeCPUDescriptorNextHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::srvComputeGPUDescriptorNextHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::uavComputeCPUDescriptorNextHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::uavComputeGPUDescriptorNextHandle = D3D12_GPU_DESCRIPTOR_HANDLE();

D3D12_CPU_DESCRIPTOR_HANDLE Shader::cbvComputeCPUDescriptorStartHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
D3D12_GPU_DESCRIPTOR_HANDLE Shader::cbvComputeGPUDescriptorStartHandle = D3D12_GPU_DESCRIPTOR_HANDLE();
weak_ptr<Camera> Shader::wpCamera;

// StreamOutput�� ���õ� ������
ParticleResource Shader::particleResource = ParticleResource();

Shader::Shader() {
	pipelineStateDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC();
	renderType = SWAP_CHAIN_RENDER;
}

Shader::~Shader() {

}

vector<weak_ptr<GameObject>>& Shader::GetGameObjects() {
	return wpGameObjects;
}

void Shader::UpdateShaderObject() {
	if (wpGameObjects.size() > 0) {
		auto removePred = [](const shared_ptr<GameObject>& pGameObject) {
			// �ش� ������Ʈ�� �̹� �����Ǿ� ���ٸ� �����̳ʿ��� �����Ѵ�.
			return !pGameObject;
		};
		wpGameObjects.erase(
			ranges::remove_if(wpGameObjects, removePred, &weak_ptr<GameObject>::lock).begin(),
			wpGameObjects.end());
	}
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

	switch (renderType) {
	case ShaderRenderType::PRE_RENDER: {
		pipelineStateDesc.NumRenderTargets = NUM_G_BUFFER;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipelineStateDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipelineStateDesc.RTVFormats[4] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipelineStateDesc.RTVFormats[5] = DXGI_FORMAT_R32_FLOAT;
		break;
	}
	case ShaderRenderType::SWAP_CHAIN_RENDER: {
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	}
	case ShaderRenderType::SHADOW_RENDER: {
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
		break;
	}
	}

	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob) {
	UINT nCompileFlag = 0;

	ComPtr<ID3DBlob> pErrorBlob;
	// ����׿� �÷���	
	nCompileFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	HRESULT hResult = D3DCompileFromFile(_fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, _shaderName.c_str(), _shaderProfile.c_str(), nCompileFlag, 0, &_pBlob, &pErrorBlob);
	char* pErrorString = NULL;
	if (pErrorBlob) {
		pErrorString = (char*)pErrorBlob->GetBufferPointer();
		cout << "����Ʈ �ڵ� ���� ���� : " << pErrorString << "\n";
	}
	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = _pBlob->GetBufferSize();
	shaderByteCode.pShaderBytecode = _pBlob->GetBufferPointer();

	return shaderByteCode;
}

// �̸� �����ϵ� ���̴��� blob�� �����Ѵ�.
D3D12_SHADER_BYTECODE Shader::LoadShaderFromFile(const wstring& _fileName, ComPtr<ID3DBlob>& _pBlob) {
	wstring path = L"Shaders/" + _fileName;
	ifstream in(path, ios::binary);
	size_t fileSize = filesystem::file_size(path);

	// ���� ũ�⸸ŭ�� blob ������ �����.
	D3DCreateBlob(fileSize, _pBlob.GetAddressOf());
	in.read((char*)_pBlob->GetBufferPointer(), fileSize);

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
	// ���̴� �� ��ũ���� ���� �ش� srv�� ����
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(_Index);

		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
		srvCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetSRVGpuDescriptorHandle(_Index, srvGPUDescriptorNextHandle);
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

		_pTexture->SetSRVGpuDescriptorHandle(i, srvGPUDescriptorNextHandle);

		srvGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = _pTexture->GetnRootParameter();
	for (int i = 0; i < nRootParameters; i++) _pTexture->SetRootParameterIndex(i, _nRootParameterStartIndex + i);
}

void Shader::CreateUnorderedAccessView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index) {
	// ���̴� �� ��ũ���� ���� �ش� uav�� ����
	// �������� ����Ÿ�, Heap ������ uav ������ üũ
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetUAVGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetUnorderedAccessViewDesc(_Index);

		_pDevice->CreateUnorderedAccessView(pShaderResource.Get(), NULL, &d3dShaderResourceViewDesc, uavCPUDescriptorNextHandle);
		uavCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetUAVGpuDescriptorHandle(_Index, uavGPUDescriptorNextHandle);
		uavGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE Shader::CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
	D3D12_GPU_DESCRIPTOR_HANDLE resourceStartHandle = srvGPUDescriptorNextHandle;
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
			// �ش� ���̴� ���� �ش� ���ҽ��� ���� srv ����
			_pDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
			srvCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
			srvGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
		}
	}
	// ù ���ҽ��� �ڵ��� ��ȯ
	return resourceStartHandle;
}

void Shader::CreateComputeShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index)
{
	// ���̴� �� ��ũ���� ���� �ش� srv�� ����
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetComputeGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(_Index);

		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvComputeCPUDescriptorNextHandle);
		srvComputeCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetSRVComputeGpuDescriptorHandle(_Index, srvComputeGPUDescriptorNextHandle);
		srvComputeGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
}

void Shader::CreateComputeShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex)
{
	srvComputeCPUDescriptorNextHandle.ptr += (::cbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);
	srvComputeGPUDescriptorNextHandle.ptr += (::cbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);

	int nTextures = _pTexture->GetnTexture();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(i);
		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvComputeCPUDescriptorNextHandle);
		srvComputeCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetSRVComputeGpuDescriptorHandle(i, srvComputeGPUDescriptorNextHandle);

		srvComputeGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = _pTexture->GetnRootParameter();
}

void Shader::CreateComputeUnorderedAccessView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index) {
	// ���̴� �� ��ũ���� ���� �ش� uav�� ����
	// �������� ����Ÿ�, Heap ������ uav ������ üũ. 
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetComputeUAVGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
				
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetUnorderedAccessViewDesc(_Index);

		_pDevice->CreateUnorderedAccessView(pShaderResource.Get(), NULL, &d3dShaderResourceViewDesc, uavComputeCPUDescriptorNextHandle);
		uavComputeCPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;

		_pTexture->SetUAVComputeGpuDescriptorHandle(_Index, uavComputeGPUDescriptorNextHandle);
		uavComputeGPUDescriptorNextHandle.ptr += ::cbvSrvDescriptorIncrementSize;
	}
}

//  StreamOutput�� ���õ� �Լ���
void Shader::AddParticle(const VS_ParticleMappedFormat& _particle) {
	UINT& nParticle = particleResource.nUploadStreamInputParticle;
	if (nParticle < particleResource.nMaxParticle) {
		//particleResource.uploadStreamInputBuffer->Map(0, NULL, (void**)&particleResource.mappedUploadStreamInputBuffer);	// ���ε� ���� ����
		memcpy(&particleResource.mappedUploadStreamInputBuffer[nParticle], &_particle, sizeof(VS_ParticleMappedFormat));
		//particleResource.uploadStreamInputBuffer->Unmap(0, NULL);	// ���ε� ���� ����
		++nParticle;
	}
}
void Shader::RenderParticle(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	size_t nStride = sizeof(VS_ParticleMappedFormat);

	particleResource.readBackBufferFilledSize->Map(0, NULL, (void**)&particleResource.mappedReadBackBufferFilledSize);
	//cout << "�̿��� ��ƼŬ �� : " << (*particleResource.mappedReadBackBufferFilledSize) / nStride << "\n";
	particleResource.nDefaultStreamOutputParticle += (*particleResource.mappedReadBackBufferFilledSize) / (UINT)nStride;
	particleResource.readBackBufferFilledSize->Unmap(0, NULL);
	
	//5. defaultDrawBuffer�� D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER�� �ٲ۴�.  defaultDrawBuffer�� �Է����� �������� �����Ѵ�.
	SynchronizeResourceTransition(_pCommandList, particleResource.defaultDrawBuffer.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	//cout << "��ƼŬ �� : " << particleResource.nDefaultStreamOutputParticle << "\n";
	if (0 < particleResource.nDefaultStreamOutputParticle) {
		static_pointer_cast<ParticleDrawShader>(gameFramework.GetShader("ParticleDrawShader"))->Render(_pCommandList);
	}

	if (1) {
		//6. �ΰ��� �����͸� �ٲ۴�. ��ƼŬ�� ������ �ٲ��ش�.
		swap(particleResource.defaultDrawBuffer, particleResource.defaultStreamInputBuffer);
		particleResource.nDefaultStreamInputParticle = particleResource.nDefaultStreamOutputParticle;	// �̹��� ��°����� ������ �Է°����� �ȴ�.
		particleResource.defaultStreamInputBufferView.BufferLocation = particleResource.defaultStreamInputBuffer->GetGPUVirtualAddress();
		particleResource.defaultDrawBufferView.BufferLocation = particleResource.defaultDrawBuffer->GetGPUVirtualAddress();
		particleResource.defaultStreamOutputBufferView.BufferLocation = particleResource.defaultDrawBuffer->GetGPUVirtualAddress();
		particleResource.defaultStreamOutputBufferView.SizeInBytes = nStride * particleResource.nMaxParticle;
	}

	//1. uploadStreamInputBuffer�� ��ƼŬ�� �߰��Ѵ�.nUploadStreamInputParticle�� �߰��� ������ŭ ������Ų��.

	//2. defaultBufferFilledSize�� uploadBufferFilledSize�� �����Ͽ� 0���� �����. �׷��� �ϴ� ��ƼŬ ������ 0���� �ʱ�ȭ
	SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
	_pCommandList->CopyResource(particleResource.defaultBufferFilledSize.Get(), particleResource.uploadBufferFilledSize.Get());
	SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);
	particleResource.nDefaultStreamOutputParticle = 0;


	//3. defaultDrawBuffer�� ���¸� D3D12_RESOURCE_STATE_STREAM_OUT�� �ٲ۴�. uploadStreamInputBuffer�� �Է����� defaultDrawBuffer�� ������� SO�ܰ踦 �����Ѵ�. 4�ܰ��� ����� ��ġ�� �ٲپ��� �� nUploadStreamInputParticle�� 0���� �ʱ�ȭ.
	bool uploadProcess = false;
	SynchronizeResourceTransition(_pCommandList, particleResource.defaultDrawBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_STREAM_OUT);
	if (0 < particleResource.nUploadStreamInputParticle) {
		static_pointer_cast<ParticleStreamOutShader>(gameFramework.GetShader("ParticleStreamOutShader"))->Render(_pCommandList, true);

		//3-2. ��Ʈ�� ����� ����� ���� ��ƼŬ�� ������ �����Ѵ�.
		SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		_pCommandList->CopyResource(particleResource.readBackBufferFilledSize.Get(), particleResource.defaultBufferFilledSize.Get());
		SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);

		particleResource.nDefaultStreamOutputParticle += particleResource.nUploadStreamInputParticle;

		particleResource.defaultStreamOutputBufferView.BufferLocation = particleResource.defaultDrawBuffer->GetGPUVirtualAddress() + nStride * particleResource.nUploadStreamInputParticle;
		particleResource.defaultStreamOutputBufferView.SizeInBytes = nStride * (particleResource.nMaxParticle - particleResource.nUploadStreamInputParticle);
		particleResource.nUploadStreamInputParticle = 0;

		uploadProcess = true;
	}

	//4. �� ���� ��ġ���� defaultStreamInputBuffer�� �̷����� defaultDrawBuffer�� ������� SO�ܰ踦 �����Ѵ�.
	bool defaultProcess = false;
	if (0 < particleResource.nDefaultStreamInputParticle) {
		static_pointer_cast<ParticleStreamOutShader>(gameFramework.GetShader("ParticleStreamOutShader"))->Render(_pCommandList, false);

		//4-2. ��Ʈ�� ����� ����� ���� ��ƼŬ�� ������ �����Ѵ�.
		SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
		_pCommandList->CopyResource(particleResource.readBackBufferFilledSize.Get(), particleResource.defaultBufferFilledSize.Get());
		SynchronizeResourceTransition(_pCommandList, particleResource.defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);


		defaultProcess = true;
	}

	_pCommandList->SOSetTargets(0, 1, NULL);	// SO����� ���� �ʵ��� �����Ѵ�. ȭ�鿡 �׷��� �ϹǷ�
}

void Shader::SetCamera(const weak_ptr<Camera>& _wpCamera) {
	wpCamera = _wpCamera;
}

void Shader::UpdateShadersObject() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetShader("BasicShader")->UpdateShaderObject();

}


void Shader::PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pPipelineState) {
		_pCommandList->SetPipelineState(pPipelineState.Get());
	}
	else {
		cout << "���������� Set ����\n";
	}

}
void Shader::AddObject(const weak_ptr<GameObject>& _pGameObject) {
	wpGameObjects.push_back(_pGameObject);
}

void Shader::ClearObject() {
	wpGameObjects.clear();
}


void Shader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	// ���̴��� ���������ο� �����Ѵ�.
	if (wpGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);

		for (auto& wpGameObject : wpGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			if (pGameObject->GetAlwaysDraw() || (pGameObject->GetSector() && pGameObject->GetSector()->GetInFrustum()))
				pGameObject->Render(_pCommandList);
		}
	}
;
}

void Shader::PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// ��Ʈ�� ���
}

void Shader::CreateConstantBufferView()
{
	// �̻��
}

void Shader::SetDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> _pCommandList) {
	_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
}



void Shader::CreateCbvSrvUavDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	_pDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&cbvSrvDescriptorHeap));

	cbvCPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cbvGPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	srvCPUDescriptorStartHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);
	srvGPUDescriptorStartHandle.ptr = cbvGPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);

	uavCPUDescriptorStartHandle.ptr = srvCPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nShaderResourceViews);
	uavGPUDescriptorStartHandle.ptr = srvGPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nShaderResourceViews);

	srvCPUDescriptorNextHandle = srvCPUDescriptorStartHandle;
	srvGPUDescriptorNextHandle = srvGPUDescriptorStartHandle;

	uavCPUDescriptorNextHandle = uavCPUDescriptorStartHandle;
	uavGPUDescriptorNextHandle = uavGPUDescriptorStartHandle;
}


void Shader::SetComputeDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> _pCommandList) {
	_pCommandList->SetDescriptorHeaps(1, computeDescriptorHeap.GetAddressOf());
}

void Shader::CreateComputeDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews) {
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	_pDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&computeDescriptorHeap));

	cbvComputeCPUDescriptorStartHandle = computeDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cbvComputeGPUDescriptorStartHandle = computeDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	
	srvComputeCPUDescriptorStartHandle.ptr = cbvComputeCPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);
	srvComputeGPUDescriptorStartHandle.ptr = cbvComputeGPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nConstantBufferViews);

	uavComputeCPUDescriptorStartHandle.ptr = srvComputeCPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nShaderResourceViews);
	uavComputeGPUDescriptorStartHandle.ptr = srvComputeGPUDescriptorStartHandle.ptr + (::cbvSrvDescriptorIncrementSize * nShaderResourceViews);

	srvComputeCPUDescriptorNextHandle = srvComputeCPUDescriptorStartHandle;
	srvComputeGPUDescriptorNextHandle = srvComputeGPUDescriptorStartHandle;

	uavComputeCPUDescriptorNextHandle = uavComputeCPUDescriptorStartHandle;
	uavComputeGPUDescriptorNextHandle = uavComputeGPUDescriptorStartHandle;
}



//////////////////// Basic Shader
BasicShader::BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "BasicShader ���� ����\n";

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
	inputElementDescs.assign(5, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

BasicWireFrameShader::BasicWireFrameShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;	// ??????????????????????????????????????????????????????????
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultWireFrameVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultWireFramePixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "BasicWireFrameShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
BasicWireFrameShader::~BasicWireFrameShader() {

}
void BasicWireFrameShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();

	auto& pGameObjects = gameFramework.GetShader("BasicShader")->GetGameObjects();

	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			if (pGameObject->GetAlwaysDraw() || (pGameObject->GetSector() && pGameObject->GetSector()->GetInFrustum())) {
				pGameObject->Render(_pCommandList);
			}
		}
	}

}
D3D12_RASTERIZER_DESC BasicWireFrameShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}
D3D12_INPUT_LAYOUT_DESC BasicWireFrameShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}
D3D12_DEPTH_STENCIL_DESC BasicWireFrameShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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


BasicShadowShader::BasicShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SHADOW_RENDER;
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultShadowVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultShadowPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "BasicShadowShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}

BasicShadowShader::~BasicShadowShader() {

}

void BasicShadowShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();

	auto& pGameObjects = gameFramework.GetShader("BasicShader")->GetGameObjects();

	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			if (pGameObject->GetAlwaysDraw() || (pGameObject->GetSector() && pGameObject->GetSector()->GetInFrustum()))
			{
				pGameObject->Render(_pCommandList);
			}
		}
	}

}

D3D12_RASTERIZER_DESC BasicShadowShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
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

D3D12_INPUT_LAYOUT_DESC BasicShadowShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC BasicShadowShader::CreateDepthStencilState() {

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
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


//////////////////// SkinnedShader
SkinnedShader::SkinnedShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "SkinnedShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
SkinnedShader::~SkinnedShader() {

}

void SkinnedShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();
	static float hitRate;

	auto& pGameObjects = gameFramework.GetShader("SkinnedShader")->GetGameObjects();
	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			auto pSkinnedGameObject = static_pointer_cast<SkinnedGameObject>(pGameObject);
			if(!pSkinnedGameObject->GetVisible()) continue;
			// 1 ~ 0 ���� �Ѱ���
			hitRate = pSkinnedGameObject->GetHitTime() * 2.f * pSkinnedGameObject->GetHit();
			_pCommandList->SetGraphicsRoot32BitConstants(13, 1, &hitRate, 0);

			// �÷��̾�� sector�� ���Ե��� �ʴ´�.
			if (!static_pointer_cast<SkinnedGameObject>(pGameObject)->GetTransparent())
			{
				pGameObject->Render(_pCommandList);
			}
		}
	}
}

D3D12_RASTERIZER_DESC SkinnedShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC SkinnedShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

SkinnedWireFrameShader::SkinnedWireFrameShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedWireFrameVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedWireFramePixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "SkinnedWireFrameShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
SkinnedWireFrameShader::~SkinnedWireFrameShader() {

}
void SkinnedWireFrameShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();
	static float hitRate;

	auto& pGameObjects = gameFramework.GetShader("SkinnedShader")->GetGameObjects();
	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			auto pSkinnedGameObject = static_pointer_cast<SkinnedGameObject>(pGameObject);
			if (!pSkinnedGameObject->GetVisible()) continue;

			// 1 ~ 0 ���� �Ѱ���
			hitRate = pSkinnedGameObject->GetHitTime() * 2.f * pSkinnedGameObject->GetHit();
			_pCommandList->SetGraphicsRoot32BitConstants(13, 1, &hitRate, 0);

			// �÷��̾�� sector�� ���Ե��� �ʴ´�.
			if (!static_pointer_cast<SkinnedGameObject>(pGameObject)->GetTransparent()) {
				pGameObject->Render(_pCommandList);
			}
		}
	}
}
D3D12_RASTERIZER_DESC SkinnedWireFrameShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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
D3D12_INPUT_LAYOUT_DESC SkinnedWireFrameShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}
D3D12_DEPTH_STENCIL_DESC SkinnedWireFrameShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

SkinnedShadowShader::SkinnedShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SHADOW_RENDER;
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedShadowVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedShadowPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "SkinnedShadowShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
SkinnedShadowShader::~SkinnedShadowShader() {

}

void SkinnedShadowShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();

	auto& pGameObjects = gameFramework.GetShader("SkinnedShader")->GetGameObjects();
	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			auto pSkinnedGameObject = static_pointer_cast<SkinnedGameObject>(pGameObject);
			if (!pSkinnedGameObject->GetVisible()) continue;
			if (!pSkinnedGameObject->GetTransparent())
				pGameObject->Render(_pCommandList);
		}
	}
}

D3D12_RASTERIZER_DESC SkinnedShadowShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC SkinnedShadowShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC SkinnedShadowShader::CreateDepthStencilState() {

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
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

SkinnedTransparentShader::SkinnedTransparentShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkinnedTransparentPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "SkinnedTransparentShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
SkinnedTransparentShader::~SkinnedTransparentShader() {

}

void SkinnedTransparentShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();

	auto& pGameObjects = gameFramework.GetShader("SkinnedShader")->GetGameObjects();
	if (pGameObjects.size() > 0) {
		if(_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			auto pSkinnedGameObject = static_pointer_cast<SkinnedGameObject>(pGameObject);
			if (!pSkinnedGameObject->GetVisible()) continue;
			if (pSkinnedGameObject->GetTransparent())
				pGameObject->Render(_pCommandList);
		}
	}
}

D3D12_RASTERIZER_DESC SkinnedTransparentShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC SkinnedTransparentShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC SkinnedTransparentShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	// ������ ������ ����� depth���� ���� �ʰ� ���纸�δ�.
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


SkinnedLobbyShader::SkinnedLobbyShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);

	// ����ü�ο� �ٷ� �׸��� Skinned Shader. �κ��
	// ������ ���̴��� ��� �������� �ſ� �����ɷ� (unroll) �̸� �����ϵ� ���̴� ������ �ҷ��� ���
	pipelineStateDesc.VS = LoadShaderFromFile(L"SkinnedLobbyShader_vs", pVSBlob);
	pipelineStateDesc.PS = LoadShaderFromFile(L"SkinnedLobbyShader_ps", pPSBlob);
	//pipelineStateDesc.VS = CompileShaderFromFile(L"SkinnedLobbyShader.hlsl", "SkinnedVertexShader", "vs_5_1", pVSBlob);
	//pipelineStateDesc.PS = CompileShaderFromFile(L"SkinnedLobbyShader.hlsl", "SkinnedLobbyPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (_hr == S_OK) cout << "SkinnedLobbyShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
SkinnedLobbyShader::~SkinnedLobbyShader() {

}

void SkinnedLobbyShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	// �̻��
}

D3D12_RASTERIZER_DESC SkinnedLobbyShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC SkinnedLobbyShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}




//////////////////// UI Shader ( 2D Shader ) 


UIShader::UIShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "Vertex2DShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "Pixel2DShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "UIShader ���� ����\n";
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

UIShader::~UIShader() {

}


D3D12_RASTERIZER_DESC UIShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

D3D12_INPUT_LAYOUT_DESC UIShader::CreateInputLayout() {
	inputElementDescs.assign(1, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();
	return inputLayoutDesc;
}


D3D12_DEPTH_STENCIL_DESC UIShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	// 2D UI�� �׻� �Ǿտ� �׷�����.
	// depth���� �Ǿտ� �� �ٸ� ������Ʈ�� �������� �ʰ� ������ �׷�����.
	// ��ư�̳� �̹��������� �׷����� ������ ���� �׻� ���� ��������.
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
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

D3D12_BLEND_DESC UIShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

//////////////// Bounding Mesh Shader ///////////////////

BoundingMeshShader::BoundingMeshShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "BoundingVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "BoundingPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "BoundingMeshShader ���� ����\n";
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

BoundingMeshShader::~BoundingMeshShader() {

}

D3D12_RASTERIZER_DESC BoundingMeshShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

D3D12_INPUT_LAYOUT_DESC BoundingMeshShader::CreateInputLayout() {
	inputElementDescs.assign(1, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

//////////////////// Instancing Shader ///////////////////////

InstancingShader::InstancingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "InstanceVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "InstancePixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "InstancingShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}

InstancingShader::~InstancingShader() {

}


void InstancingShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	// ���̴��� ���������ο� �����Ѵ�.
	GameObject::RenderInstanceObjects(_pCommandList);
}


D3D12_RASTERIZER_DESC InstancingShader::CreateRasterizerState() {
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

D3D12_INPUT_LAYOUT_DESC InstancingShader::CreateInputLayout() {
	inputElementDescs.assign(9, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	inputElementDescs[5] = { "WORLDMAT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "WORLDMAT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[7] = { "WORLDMAT", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[8] = { "WORLDMAT", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

InstancingWireFrameShader::InstancingWireFrameShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "InstanceWireFrameVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "InstanceWireFramePixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "InstancingWireFrameShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}
InstancingWireFrameShader::~InstancingWireFrameShader() {

}
void InstancingWireFrameShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	// ���̴��� ���������ο� �����Ѵ�.
	GameObject::RenderWireFrameInstanceObjects(_pCommandList);
}
D3D12_RASTERIZER_DESC InstancingWireFrameShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}
D3D12_INPUT_LAYOUT_DESC InstancingWireFrameShader::CreateInputLayout() {
	inputElementDescs.assign(9, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	inputElementDescs[5] = { "WORLDMAT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "WORLDMAT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[7] = { "WORLDMAT", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[8] = { "WORLDMAT", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}
D3D12_DEPTH_STENCIL_DESC InstancingWireFrameShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

InstancingShadowShader::InstancingShadowShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "InstanceShadowVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "InstanceShadowPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "InstancingShadowShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();
	inputElementDescs.clear();
}

InstancingShadowShader::~InstancingShadowShader() {

}

void InstancingShadowShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameObject::RenderShadowInstanceObjects(_pCommandList);
}

D3D12_RASTERIZER_DESC InstancingShadowShader::CreateRasterizerState() {
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

D3D12_INPUT_LAYOUT_DESC InstancingShadowShader::CreateInputLayout() {
	inputElementDescs.assign(9, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	inputElementDescs[5] = { "WORLDMAT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "WORLDMAT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[7] = { "WORLDMAT", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[8] = { "WORLDMAT", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC InstancingShadowShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
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



//////////////////// Blending Shader ///////////////////////

BlendingShader::BlendingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "BlendingShader ���� ����\n";
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

BlendingShader::~BlendingShader() {

}


D3D12_RASTERIZER_DESC BlendingShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC BlendingShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };



	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_BLEND_DESC BlendingShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC BlendingShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

void BlendingShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	XMFLOAT3 cameraPos = wpCamera.lock()->GetWorldPosition();

	// ī�޶���� �Ÿ��� ���Ͽ� �ָ� �ִ� ������Ʈ�� ���� �׸���.
	auto func = [cameraPos](const shared_ptr<GameObject>& _a, const shared_ptr<GameObject>& _b) {
		if (!_a) return false;
		if (!_b) return true;
		return Vector3::LengthSq(Vector3::Subtract(cameraPos, _a->GetWorldPosition())) > Vector3::LengthSq(Vector3::Subtract(cameraPos, _b->GetWorldPosition()));
	};
	
	ranges::sort(wpGameObjects, func, &weak_ptr<GameObject>::lock);

	Shader::Render(_pCommandList, _setPipeline);
}

//////////////////// Effect Shader ///////////////////////


EffectShader::EffectShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "EffectVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "EffectPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "EffectShader ���� ����\n";
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

EffectShader::~EffectShader() {

}


D3D12_RASTERIZER_DESC EffectShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC EffectShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_BLEND_DESC EffectShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC EffectShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

void EffectShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	XMFLOAT3 cameraPos = wpCamera.lock()->GetWorldPosition();

	// ī�޶���� �Ÿ��� ���Ͽ� �ָ� �ִ� ������Ʈ�� ���� �׸���.
	auto func = [cameraPos](const shared_ptr<GameObject>& _a, const shared_ptr<GameObject>& _b) {
		// Effect�� ��� ������ ������Ʈ�� ���� �� �����Ƿ� �� ��� �ڷ� �����ش�.
		if (!_a) return false;
		if (!_b) return true;
		return Vector3::LengthSq(Vector3::Subtract(cameraPos, _a->GetWorldPosition())) > Vector3::LengthSq(Vector3::Subtract(cameraPos, _b->GetWorldPosition()));
	};
	ranges::sort(wpGameObjects, func, &weak_ptr<GameObject>::lock);

	Shader::Render(_pCommandList, _setPipeline);
}


//////////////////// Billboard Shader ///////////////////////


BillboardShader::BillboardShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.GS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardGeometryShader", "gs_5_1", pGSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "BillboardShader ���� ����\n";
	else cout << hr << "\n";
	pVSBlob.Reset();
	pGSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

BillboardShader::~BillboardShader() {

}


D3D12_RASTERIZER_DESC BillboardShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC BillboardShader::CreateInputLayout() {
	inputElementDescs.assign(1, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC BillboardShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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

void BillboardShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	GameFramework& gameFramework = GameFramework::Instance();

	auto& pGameObjects = gameFramework.GetShader("BillboardShader")->GetGameObjects();

	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	if (pGameObjects.size() > 0) {
		if (_setPipeline) PrepareRender(_pCommandList);
		for (auto& wpGameObject : pGameObjects) {
			if (wpGameObject.expired()) continue;
			auto pGameObject = wpGameObject.lock();
			if (pGameObject->GetAlwaysDraw() || (pGameObject->GetSector() && pGameObject->GetSector()->GetInFrustum())) {
				pGameObject->Render(_pCommandList);
			}
		}
	}

	Shader::Render(_pCommandList, _setPipeline);
}


//////////////////// SkyBox Shader ///////////////////////

SkyBoxShader::SkyBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkyBoxVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkyBoxPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "SkyBoxShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

SkyBoxShader::~SkyBoxShader() {

}

D3D12_RASTERIZER_DESC SkyBoxShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_INPUT_LAYOUT_DESC SkyBoxShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	// ���̰˻縦 �Ͽ� �� �ڿ� �׷������� �Ѵ�. 
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc =		D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

D3D12_BLEND_DESC SkyBoxShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	

	return blendDesc;
}


LightingShader::LightingShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);

	// ������ ���̴��� ��� �������� �ſ� �����ɷ� (unroll) �̸� �����ϵ� ���̴� ������ �ҷ��� ���
	pipelineStateDesc.VS = LoadShaderFromFile(L"DeferredLighting_vs", pVSBlob);
	pipelineStateDesc.PS = LoadShaderFromFile(L"DeferredLighting_ps", pPSBlob);
	//pipelineStateDesc.VS = CompileShaderFromFile(L"DeferredLighting.hlsl", "LightingVertexShader", "vs_5_1", pVSBlob);
	//pipelineStateDesc.PS = CompileShaderFromFile(L"DeferredLighting.hlsl", "LightingPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "LightingShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

LightingShader::~LightingShader() {

}

D3D12_RASTERIZER_DESC LightingShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 2.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC LightingShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



/////////////////////////    Shader Manager   ////////////////////////////////

bool ShaderManager::InitShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature, const ComPtr<ID3D12RootSignature>& _pComputeRootSignature)
{
	shared_ptr<Shader> basicShader = make_shared<BasicShader>(_pDevice, _pRootSignature);
	if (basicShader) storage["BasicShader"] = basicShader;
	else return false;

	shared_ptr<Shader> basicWireFrameShader = make_shared<BasicWireFrameShader>(_pDevice, _pRootSignature);
	if (basicWireFrameShader) storage["BasicWireFrameShader"] = basicWireFrameShader;
	else return false;

	shared_ptr<Shader> basicShadowShader = make_shared<BasicShadowShader>(_pDevice, _pRootSignature);
	if (basicShadowShader) storage["BasicShadowShader"] = basicShadowShader;
	else return false;

	shared_ptr<Shader> uiShader = make_shared<UIShader>(_pDevice, _pRootSignature);
	if (uiShader) storage["UIShader"] = uiShader;
	else return false;
	
	shared_ptr<Shader> boundingShader = make_shared<BoundingMeshShader>(_pDevice, _pRootSignature);
	if (boundingShader) storage["BoundingMeshShader"] = boundingShader;
	else return false;

	shared_ptr<Shader> instancingShader = make_shared<InstancingShader>(_pDevice, _pRootSignature);
	if (instancingShader) storage["InstancingShader"] = instancingShader;
	else return false;

	shared_ptr<Shader> instancingWireFrameShader = make_shared<InstancingWireFrameShader>(_pDevice, _pRootSignature);
	if (instancingWireFrameShader) storage["InstancingWireFrameShader"] = instancingWireFrameShader;
	else return false;

	shared_ptr<Shader> instancingShadowShader = make_shared<InstancingShadowShader>(_pDevice, _pRootSignature);
	if (instancingShadowShader) storage["InstancingShadowShader"] = instancingShadowShader;
	else return false;

	shared_ptr<Shader> skinnedTransparentShader = make_shared<SkinnedTransparentShader>(_pDevice, _pRootSignature);
	if (skinnedTransparentShader) storage["SkinnedTransparentShader"] = skinnedTransparentShader;
	else return false;

	shared_ptr<Shader> skinnedShader = make_shared<SkinnedShader>(_pDevice, _pRootSignature);
	if (skinnedShader) storage["SkinnedShader"] = skinnedShader;
	else return false;

	shared_ptr<Shader> skinnedWireFrameShader = make_shared<SkinnedWireFrameShader>(_pDevice, _pRootSignature);
	if (skinnedWireFrameShader) storage["SkinnedWireFrameShader"] = skinnedWireFrameShader;
	else return false;

	shared_ptr<Shader> skinnedShadowShader = make_shared<SkinnedShadowShader>(_pDevice, _pRootSignature);
	if (skinnedShadowShader) storage["SkinnedShadowShader"] = skinnedShadowShader;
	else return false;

	shared_ptr<Shader> skinnedLobbyShader = make_shared<SkinnedLobbyShader>(_pDevice, _pRootSignature);
	if (skinnedLobbyShader) storage["SkinnedLobbyShader"] = skinnedLobbyShader;
	else return false;

	shared_ptr<Shader> blendingShader = make_shared<BlendingShader>(_pDevice, _pRootSignature);
	if (blendingShader) storage["BlendingShader"] = blendingShader;
	else return false;

	shared_ptr<Shader> effectShader = make_shared<EffectShader>(_pDevice, _pRootSignature);
	if (effectShader) storage["EffectShader"] = effectShader;
	else return false;

	shared_ptr<Shader> billboardShader = make_shared<BillboardShader>(_pDevice, _pRootSignature);
	if (billboardShader) storage["BillboardShader"] = billboardShader;
	else return false;

	shared_ptr<Shader> skyBoxShader = make_shared<SkyBoxShader>(_pDevice, _pRootSignature);
	if (skyBoxShader) storage["SkyBoxShader"] = skyBoxShader;
	else return false;

	shared_ptr<Shader> lightingShader = make_shared<LightingShader>(_pDevice, _pRootSignature);
	if (lightingShader) storage["LightingShader"] = lightingShader;
	else return false;

	shared_ptr<Shader> particleDrawShader = make_shared<ParticleDrawShader>(_pDevice, _pRootSignature);
	if (particleDrawShader) storage["ParticleDrawShader"] = particleDrawShader;
	else return false;

	shared_ptr<Shader> particleStreamOutShader = make_shared<ParticleStreamOutShader>(_pDevice, _pRootSignature);
	if (particleStreamOutShader) storage["ParticleStreamOutShader"] = particleStreamOutShader;
	else return false;

	shared_ptr<Shader> blurComputeShader = make_shared<BlurComputeShader>(_pDevice, _pComputeRootSignature);
	if (blurComputeShader) storage["BlurComputeShader"] = blurComputeShader;
	else return false;

	shared_ptr<Shader> shadowComputeShader = make_shared<ShadowComputeShader>(_pDevice, _pComputeRootSignature);
	if (shadowComputeShader) storage["ShadowComputeShader"] = shadowComputeShader;
	else return false;

	shared_ptr<Shader> postShader = make_shared<PostShader>(_pDevice, _pRootSignature);
	if (postShader) storage["PostShader"] = postShader;
	else return false;

	// ���Ŀ� ��� �߰�
	return true;
}

shared_ptr<Shader> ShaderManager::GetShader(const string& _name)
{
	if (storage[_name])
		return storage[_name];

	else {
		cout << "�ش� ���̴��� �������� ����\n";
		return nullptr;
	}
	
}

///////////////////////////////////////////////////////////////////////////////

void ParticleResource::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256�� ���
	// 256�� ����� �ȸ���?

	// ��Ʈ�� �ƿ�ǲ ���ҽ� ����
	UINT nStride = (UINT)(sizeof(VS_ParticleMappedFormat));
	nDefaultStreamInputParticle = 0;
	nUploadStreamInputParticle = 0;

	// ��ƼŬ�� ���� ������ �� ���ҽ����� ����
	uploadStreamInputBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, nStride * nMaxParticle, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	defaultStreamInputBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, nStride * nMaxParticle, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	defaultDrawBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, nStride * nMaxParticle, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	// ��ƼŬ�� ������ ����/����/�б����� ���ҽ� ����
	defaultBufferFilledSize = ::CreateBufferResource(_pDevice, _pCommandList, NULL, sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);
	uploadBufferFilledSize = ::CreateBufferResource(_pDevice, _pCommandList, &nDefaultStreamInputParticle, sizeof(UINT), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COPY_SOURCE, NULL);
	readBackBufferFilledSize = ::CreateBufferResource(_pDevice, _pCommandList, NULL, sizeof(UINT), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);

	// ���ε� ��ǲ ���� �� ���� �� ����
	uploadStreamInputBuffer->Map(0, NULL, (void**)&mappedUploadStreamInputBuffer);	// ���ε� ���� ����
	//uploadStreamInputBuffer->Unmap(0, NULL);	// ���ε� ���� ����
	uploadStreamInputBufferView.BufferLocation = uploadStreamInputBuffer->GetGPUVirtualAddress();
	uploadStreamInputBufferView.SizeInBytes = nStride * nMaxParticle;
	uploadStreamInputBufferView.StrideInBytes = nStride;

	// ����Ʈ ��ǲ ���� �� ����
	defaultStreamInputBufferView.BufferLocation = defaultStreamInputBuffer->GetGPUVirtualAddress();
	defaultStreamInputBufferView.SizeInBytes = nStride * nMaxParticle;
	defaultStreamInputBufferView.StrideInBytes = nStride;

	// ����Ʈ �ƿ�ǲ ���� �� ����
	defaultStreamOutputBufferView.BufferLocation = defaultDrawBuffer->GetGPUVirtualAddress();
	defaultStreamOutputBufferView.SizeInBytes = nStride * nMaxParticle;
	defaultStreamOutputBufferView.BufferFilledSizeLocation = defaultBufferFilledSize->GetGPUVirtualAddress();

	// �������ϱ����� ���� �� ����
	defaultDrawBufferView.BufferLocation = defaultDrawBuffer->GetGPUVirtualAddress();
	defaultDrawBufferView.SizeInBytes = nStride * nMaxParticle;
	defaultDrawBufferView.StrideInBytes = nStride;

	// ��ƼŬ�� ������ �б����� ���ҽ��� ����
	readBackBufferFilledSize->Map(0, NULL, (void**)&mappedReadBackBufferFilledSize);
	readBackBufferFilledSize->Unmap(0, NULL);

	// �ʱ� ���� ����
	SynchronizeResourceTransition(_pCommandList, defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_DEST);
	_pCommandList->CopyResource(defaultBufferFilledSize.Get(), uploadBufferFilledSize.Get());
	SynchronizeResourceTransition(_pCommandList, defaultBufferFilledSize.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_STREAM_OUT);

	// ���̴��� �ؽ��� ��������
	texture = GameFramework::Instance().GetTextureManager().GetTexture("ParticleTexture", _pDevice, _pCommandList, 4);
}

/////////////////// Particle Shader
////////// Streamoutput
ParticleStreamOutShader::ParticleStreamOutShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleStreamOutVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.GS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleStreamOutGeometryShader", "gs_5_1", pGSBlob);
	pipelineStateDesc.StreamOutput = CreateStreamOuputState();

	// PS�� ������ �ʿ䰡 ����.
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;
	pipelineStateDesc.PS = d3dShaderByteCode;

	HRESULT hResult = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)pPipelineState.GetAddressOf());
	if (hResult != S_OK)
		cout << "Streamoutput ���������� ���� ����\n";

	pVSBlob.Reset();
	pGSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}
ParticleStreamOutShader::~ParticleStreamOutShader() {

}

D3D12_RASTERIZER_DESC ParticleStreamOutShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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
D3D12_INPUT_LAYOUT_DESC ParticleStreamOutShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	// �ø�ƽ, �ø�ƽ �ε���, ����, ��ǲ����, ������, ��ǲ����Ŭ����, InstaneDataStepRate : https://vitacpp.tistory.com/m/70
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float3
	inputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float3
	inputElementDescs[2] = { "BOARDSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float2
	inputElementDescs[3] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float
	inputElementDescs[4] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// uint

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}
D3D12_DEPTH_STENCIL_DESC ParticleStreamOutShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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
D3D12_STREAM_OUTPUT_DESC ParticleStreamOutShader::CreateStreamOuputState() {
	D3D12_STREAM_OUTPUT_DESC streamOutputDesc;
	::ZeroMemory(&streamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));

	UINT nStreamOutputDecls = 5;
	D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
	pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
	pd3dStreamOutputDecls[2] = { 0, "BOARDSIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDecls[3] = { 0, "LIFETIME", 0, 0, 1, 0 };
	pd3dStreamOutputDecls[4] = { 0, "PARTICLETYPE", 0, 0, 1, 0 };

	UINT* pBufferStrides = new UINT[1];
	pBufferStrides[0] = sizeof(VS_ParticleMappedFormat);

	streamOutputDesc.NumEntries = nStreamOutputDecls;
	streamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
	streamOutputDesc.NumStrides = 1;
	streamOutputDesc.pBufferStrides = pBufferStrides;
	streamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	return(streamOutputDesc);
}

void ParticleStreamOutShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool isUploadInput) {

	PrepareRender(_pCommandList);
	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	D3D12_STREAM_OUTPUT_BUFFER_VIEW streamOutputBufferViews[1] = { Shader::particleResource.defaultStreamOutputBufferView };
	_pCommandList->SOSetTargets(0, 1, streamOutputBufferViews);

	if (isUploadInput) {	// ���ε� �� ���ҽ��� �Է����� SO�ܰ踦 ������ ���
		D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { Shader::particleResource.uploadStreamInputBufferView };
		_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);

		_pCommandList->DrawInstanced((UINT)Shader::particleResource.nUploadStreamInputParticle, 1, 0, 0);
	}
	else {	// ����Ʈ �� ���ҽ��� �Է����� SO�ܰ踦 ������ ���
		D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { Shader::particleResource.defaultStreamInputBufferView };
		_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);

		_pCommandList->DrawInstanced((UINT)Shader::particleResource.nDefaultStreamInputParticle, 1, 0, 0);
	}
}

////////// Draw
ParticleDrawShader::ParticleDrawShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	renderType = ShaderRenderType::PRE_RENDER;
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleDrawVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.GS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleDrawGeometryShader", "gs_5_1", pGSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleDrawPixelShader", "ps_5_1", pPSBlob);

	HRESULT hResult = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)pPipelineState.GetAddressOf());

	pVSBlob.Reset();
	pGSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}
ParticleDrawShader::~ParticleDrawShader() {

}

D3D12_RASTERIZER_DESC ParticleDrawShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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
D3D12_INPUT_LAYOUT_DESC ParticleDrawShader::CreateInputLayout() {
	inputElementDescs.assign(5, {});

	// �ø�ƽ, �ø�ƽ �ε���, ����, ��ǲ����, ������, ��ǲ����Ŭ����, InstaneDataStepRate : https://vitacpp.tistory.com/m/70
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float3
	inputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float3
	inputElementDescs[2] = { "BOARDSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float2
	inputElementDescs[3] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// float
	inputElementDescs[4] = { "PARTICLETYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };	// uint

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

void ParticleDrawShader::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _setPipeline) {
	if(_setPipeline) PrepareRender(_pCommandList);
	particleResource.texture->UpdateShaderVariable(_pCommandList);

	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	_pCommandList->SOSetTargets(0, 1, NULL);	// SO����� ���� �ʵ��� �����Ѵ�. ȭ�鿡 �׷��� �ϹǷ�

	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { Shader::particleResource.defaultDrawBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);

	_pCommandList->DrawInstanced((UINT)Shader::particleResource.nDefaultStreamOutputParticle, 1, 0, 0);
}

BlurComputeShader::BlurComputeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };

	ZeroMemory(&computePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	computePipelineStateDesc.pRootSignature = _pRootSignature.Get();
	computePipelineStateDesc.CS = CompileShaderFromFile(L"ComputeShader.hlsl", "radarResult", "cs_5_1", pCSBlob);
	computePipelineStateDesc.NodeMask = 0;
	computePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	computePipelineStateDesc.CachedPSO = d3dCachedPipelineState;

	HRESULT _hr = _pDevice->CreateComputePipelineState(&computePipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (!FAILED(_hr)) cout << "BlurComputeShader ���� ����\n";
	
	numThreads = XMUINT3(ceil((float)C_WIDTH / 32.0f), ceil((float)C_HEIGHT / 32.0f), 1);

}

BlurComputeShader::~BlurComputeShader() {

}

void BlurComputeShader::Dispatch(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	PrepareRender(_pCommandList);


	// ��ó���� �Ϸ�� ������� 0������ ����
	gameFramework.GetPostBuffer()->UpdateComputeShaderVariable(_pCommandList, 0, -1);

	//���̰��� ��� �ؽ�ó�� srv 1���� ����
	gameFramework.GetGBuffer()->UpdateComputeShaderVariable(_pCommandList, 1, -1);

	// RWTexture�� uav 2���� ����
	gameFramework.GetComputeBuffer()->UpdateComputeShaderVariable(_pCommandList, -1, 2);

	// ���̴� �ؽ�ó�� �����Ѵ�.
	gameFramework.GetWireFrameMap()->UpdateComputeShaderVariable(_pCommandList, 3, -1);
	
	// ��ǻƮ ���̴��� �����Ѵ�.
	_pCommandList->Dispatch(numThreads.x, numThreads.y, 1);

}

D3D12_RASTERIZER_DESC BlurComputeShader::CreateRasterizerState() {
	return D3D12_RASTERIZER_DESC();
}

D3D12_INPUT_LAYOUT_DESC BlurComputeShader::CreateInputLayout()
{
	return D3D12_INPUT_LAYOUT_DESC();
}


ShadowComputeShader::ShadowComputeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };

	ZeroMemory(&computePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	computePipelineStateDesc.pRootSignature = _pRootSignature.Get();
	computePipelineStateDesc.CS = CompileShaderFromFile(L"ComputeShader.hlsl", "MergeShadow", "cs_5_1", pCSBlob);
	computePipelineStateDesc.NodeMask = 0;
	computePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	computePipelineStateDesc.CachedPSO = d3dCachedPipelineState;

	HRESULT _hr = _pDevice->CreateComputePipelineState(&computePipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (!FAILED(_hr)) cout << "ShadowComputeShader ���� ����\n";

	numThreads = XMUINT3(ceil((float)C_WIDTH / 32.0f), ceil((float)C_HEIGHT / 32.0f), 1);

}

ShadowComputeShader::~ShadowComputeShader() {

}

void ShadowComputeShader::Dispatch(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// ��ǻƮ ���̴��� �����Ѵ�.
	_pCommandList->Dispatch(numThreads.x, numThreads.y, 1);

}

D3D12_RASTERIZER_DESC ShadowComputeShader::CreateRasterizerState() {
	return D3D12_RASTERIZER_DESC();
}

D3D12_INPUT_LAYOUT_DESC ShadowComputeShader::CreateInputLayout()
{
	return D3D12_INPUT_LAYOUT_DESC();
}



PostShader::PostShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	renderType = ShaderRenderType::SWAP_CHAIN_RENDER;
	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "PostVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "PostPixelShader", "ps_5_1", pPSBlob);

	HRESULT hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	if (hr == S_OK) cout << "PostShader ���� ����\n";

	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

PostShader::~PostShader() {

}

D3D12_RASTERIZER_DESC PostShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 2.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC PostShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

