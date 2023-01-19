#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"

Camera::Camera() {
	viewTransform = Matrix4x4::Identity();
	projectionTransform = Matrix4x4::Identity();
	viewPort = { 0,0, C_WIDTH, C_HEIGHT, 0, 1 };
	scissorRect = { 0,0, C_WIDTH, C_HEIGHT };

}

Camera::~Camera() {
	pCameraBuffer->Unmap(0, NULL);
}

void Camera::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create();
	GameFramework& gameFramework = GameFramework::Instance();	// gameFramework의 래퍼런스를 가져온다.

	name = "카메라";


	auto [width, height] = gameFramework.GetClientSize();
	viewPort = { 0,0, (float)width, (float)height, 0, 1 };
	scissorRect = { 0,0, width, height };

	UINT cbElementSize = (sizeof(VS_CameraMappedFormat) + 255) & (~255);

	ComPtr<ID3D12Resource> temp;
	pCameraBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pCameraBuffer->Map(0, NULL, (void**)&pMappedCamera);

	//UpdateViewTransform();
	UpdateProjectionTransform(0.1f, 10.0f, float(width) / height, 90.0f);
	pBoundingFrustum = make_shared<BoundingFrustum>(XMLoadFloat4x4(&projectionTransform));

}

void Camera::SetViewPortAndScissorRect(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&viewTransform)));	// 쉐이더는 열?우선 행렬이기 때문에 전치행렬로 바꾸어서 보내준다.
	memcpy(&pMappedCamera->view, &view, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(XMLoadFloat4x4(&projectionTransform)));	// 쉐이더는 열?우선 행렬이기 때문에 전치행렬로 바꾸어서 보내준다.
	memcpy(&pMappedCamera->projection, &projection, sizeof(XMFLOAT4X4));

	XMFLOAT3 worldPosition = GetWorldPosition();
	memcpy(&pMappedCamera->position, &worldPosition, sizeof(XMFLOAT3));

	XMFLOAT3 rr = playerPosition;
	memcpy(&pMappedCamera->playerPosition, &rr, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pCameraBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

void Camera::UpdateViewTransform() {
	// 플레이어 위치에서 각방향으로 바라보고 있을때의 뷰행렬을 저장
	XMFLOAT3 worldPosition = GetWorldPosition();

	XMFLOAT3 lookAtWorld = Vector3::Add(worldPosition, GetWorldLookVector());
	viewTransform = Matrix4x4::LookAtLH(worldPosition, lookAtWorld, GetWorldUpVector());
}

void Camera::UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle) {
	projectionTransform = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(_fovAngle), _aspectRatio, _nearDistance, _farDistance);
}

void Camera::UpdateWorldTransform() {
	GameObject::UpdateWorldTransform();
	UpdateViewTransform();
}
void Camera::UpdateObject() {

	// 미터 단위
	float bias = 5.0f;

	GameObject::UpdateObject();

	// 현재 투영 행렬로 바운딩 프러스텀을 만든다.
	BoundingFrustum b(XMLoadFloat4x4(&projectionTransform));

	// 현재 카메라의 이동, 회전을 프러스텀에 적용한다.
	b.Transform(*pBoundingFrustum, XMLoadFloat4x4(&worldTransform));

	// 여러 섹터에 걸치는 오브젝트들이 그려지지 않는 것을 막기 위해 기존 프러스텀보다 살짝 더 크게 만든다.
	pBoundingFrustum->Origin = Vector3::Subtract(pBoundingFrustum->Origin, Vector3::ScalarProduct(GetWorldLookVector(), bias));
	pBoundingFrustum->Far += bias * 2.f;
}