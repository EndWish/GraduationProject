#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"	// ������ü�� �����Ͽ� Scene�� ������Ʈ�� �����ϱ� ���� �����Ͽ���.

/// �������� ��� �Լ���
// ������, �Ҹ���
Camera::Camera(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList)
{
	m_lookAtWorld = XMFLOAT3(0, 0, 0);
	m_offset = XMFLOAT3(0, 0, 0);

	Matrix4x4::Identity(m_viewTransform);
	Matrix4x4::Identity(m_projectionTransform);

	auto[windowClientWidth, windowClientHeight] = GameFramework::Instance().GetWindowClientSize();
	m_viewport = { 0, 0, (float)windowClientWidth , (float)windowClientHeight, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, windowClientWidth , windowClientHeight };

	CreateShaderVariables(pDevice, pCommandList);
}
Camera::~Camera()
{
	m_pCameraTransformBuffer->Unmap(0, NULL);
}

// ī�޶� ��ȯ�� �ʿ��� �������� ���� ���ε����(���ҽ�) ���� �� ����
void Camera::CreateShaderVariables(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList) {
	
	UINT bufferBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256�� ���
	m_pCameraTransformBuffer = ::CreateBufferResource(pDevice, pCommandList, NULL, bufferBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);	// UPLOAD ���� CPU���� ���ҽ��� ������ �� ����
	m_pCameraTransformBuffer->Map(0, NULL, (void**)&m_pMappedCameraTransformBuffer);	// UPLOAD ���� ���������� ������ �����ϴ�.
}

/// ��� �Լ���
// get set �Լ�
void Camera::SetViewportsAndScissorRects(const ComPtr<ID3D12GraphicsCommandList>& pCommandList) 
{
	pCommandList->RSSetViewports(1, &m_viewport);
	pCommandList->RSSetScissorRects(1, &m_scissorRect);
}
void Camera::SetOffset(const XMFLOAT3& offset) 
{
	m_offset = offset;
}

// ��ĺ�ȯ ���� �Լ�
void Camera::GenerateViewMatrix() 
{
	XMFLOAT3 position = Vector3::TransformCoord(m_offset, m_worldTransform);
	m_viewTransform = Matrix4x4::LookAtLH(position, m_lookAtWorld, GetWorldUpVector());
}
void Camera::GenerateViewMatrix(const XMFLOAT3& lookAtWorld)
{
	m_lookAtWorld = lookAtWorld;
	GenerateViewMatrix();
}
void Camera::GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float FOVAngle) 
{
	m_projectionTransform = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(FOVAngle), aspectRatio, nearPlaneDistance, farPlaneDistance);
}
void Camera::UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& pCommandList) {

	// �亯ȯ ����� ���ҽ��� write
	XMFLOAT4X4 viewTransform;
	XMStoreFloat4x4(&viewTransform, XMMatrixTranspose(XMLoadFloat4x4(&m_viewTransform)));			// ���̴��� ��-�켱 ���(����� ���� �������Ϳ� �����)�̱� ������ ��ġ��ķ� �ٲ㼭 �����͸� �ش�.
	::memcpy(&m_pMappedCameraTransformBuffer->m_viewTransform, &viewTransform, sizeof(XMFLOAT4X4));	// ���ε� ������ ���� ���ҽ��� ���� �ٲ۴�.

	// ������ȯ ����� ���ҽ��� write
	XMFLOAT4X4 projectionTransform;
	XMStoreFloat4x4(&projectionTransform, XMMatrixTranspose(XMLoadFloat4x4(&m_projectionTransform)));
	::memcpy(&m_pMappedCameraTransformBuffer->m_projectionTransform, &projectionTransform, sizeof(XMFLOAT4X4));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = m_pCameraTransformBuffer->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

