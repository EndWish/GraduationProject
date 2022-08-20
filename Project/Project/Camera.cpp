#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"

Camera::Camera() {

}

Camera::~Camera() {

}

void Camera::Create() {
	GameObject::Create();
	GameFramework& gameFramework = GameFramework::Instance();	// gameFramework�� ���۷����� �����´�.

	name = "ī�޶�";

	viewTransform = Matrix4x4::Identity();
	projectionTransform = Matrix4x4::Identity();

	auto [width, height] = gameFramework.GetClientSize();
	viewPort = { 0,0, (float)width, (float)height, 0, 1 };
	scissorRect = { 0,0, width, height };

	UINT cbElementSize = (sizeof(VS_CameraMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;
	pCameraBuffer = CreateBufferResource(gameFramework.GetDevice(), gameFramework.GetCommandList(), NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pCameraBuffer->Map(0, NULL, (void**)&pMappedCamera);

	UpdateViewTransform();
	UpdateProjectionTransform(0.1f, 1000.0f, (float)width / height, 75.0f);
}

void Camera::SetViewPortAndScissorRect() {
	GameFramework& gameFramework = GameFramework::Instance();
	const ComPtr<ID3D12GraphicsCommandList>& pCommandList = gameFramework.GetCommandList();
	pCommandList->RSSetViewports(1, &viewPort);
	pCommandList->RSSetScissorRects(1, &scissorRect);
}

void Camera::UpdateShaderVariable() {
	GameFramework& gameFramework = GameFramework::Instance();
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&viewTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->view, &view, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(XMLoadFloat4x4(&projectionTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->projection, &projection, sizeof(XMFLOAT4X4));

	XMFLOAT3 worldPosition = GetWorldPosition();
	memcpy(&pMappedCamera->position, &worldPosition, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pCameraBuffer->GetGPUVirtualAddress();
	gameFramework.GetCommandList()->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

void Camera::UpdateViewTransform() {
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