#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"	// 상위객체에 접근하여 Scene에 오브젝트를 연결하기 위해 포함하였다.

/// 생성관련 멤버 함수▼
// 생성자, 소멸자
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

// 카메라 변환에 필요한 변수들을 담을 업로드버퍼(리소스) 생성 및 매핑
void Camera::CreateShaderVariables(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList) {
	
	UINT bufferBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pCameraTransformBuffer = ::CreateBufferResource(pDevice, pCommandList, NULL, bufferBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);	// UPLOAD 힙은 CPU에서 리소스를 갱신할 수 있음
	m_pCameraTransformBuffer->Map(0, NULL, (void**)&m_pMappedCameraTransformBuffer);	// UPLOAD 힙은 영구적으로 매핑이 가능하다.
}

/// 멤버 함수▼
// get set 함수
void Camera::SetViewportsAndScissorRects(const ComPtr<ID3D12GraphicsCommandList>& pCommandList) 
{
	pCommandList->RSSetViewports(1, &m_viewport);
	pCommandList->RSSetScissorRects(1, &m_scissorRect);
}
void Camera::SetOffset(const XMFLOAT3& offset) 
{
	m_offset = offset;
}

// 행렬변환 관련 함수
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

	// 뷰변환 행렬을 리소스에 write
	XMFLOAT4X4 viewTransform;
	XMStoreFloat4x4(&viewTransform, XMMatrixTranspose(XMLoadFloat4x4(&m_viewTransform)));			// 쉐이더는 열-우선 행렬(행렬의 열이 레지스터에 저장됨)이기 때문에 전치행렬로 바꿔서 데이터를 준다.
	::memcpy(&m_pMappedCameraTransformBuffer->m_viewTransform, &viewTransform, sizeof(XMFLOAT4X4));	// 매핑된 변수를 통해 리소스의 값을 바꾼다.

	// 투영변환 행렬을 리소스에 write
	XMFLOAT4X4 projectionTransform;
	XMStoreFloat4x4(&projectionTransform, XMMatrixTranspose(XMLoadFloat4x4(&m_projectionTransform)));
	::memcpy(&m_pMappedCameraTransformBuffer->m_projectionTransform, &projectionTransform, sizeof(XMFLOAT4X4));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = m_pCameraTransformBuffer->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

