#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"

Camera::Camera() {
	viewTransform = Matrix4x4::Identity();
	projectionTransform = Matrix4x4::Identity();
	viewPort = { 0,0, C_WIDTH, C_HEIGHT, 0, 1 };
	scissorRect = { 0,0, C_WIDTH, C_HEIGHT };

	localOffset = XMFLOAT3(0, 0, 0);
	minDistance = 0.3f;
	maxDistance = 3.f;

}

Camera::~Camera() {
	pCameraBuffer->Unmap(0, NULL);
}

void Camera::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create();
	GameFramework& gameFramework = GameFramework::Instance();	// gameFramework�� ���۷����� �����´�.

	name = "ī�޶�";


	auto [width, height] = gameFramework.GetClientSize();
	viewPort = { 0,0, (float)width, (float)height, 0, 1 };
	scissorRect = { 0,0, width, height };

	UINT cbElementSize = (sizeof(VS_CameraMappedFormat) + 255) & (~255);

	ComPtr<ID3D12Resource> temp;
	pCameraBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pCameraBuffer->Map(0, NULL, (void**)&pMappedCamera);

	//UpdateViewTransform();
	UpdateProjectionTransform(0.1f, 100.0f, float(width) / height, 65.0f);
	pBoundingFrustum = make_shared<BoundingFrustum>(XMLoadFloat4x4(&projectionTransform));

}

void Camera::SetViewPortAndScissorRect(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&viewTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->view, &view, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(XMLoadFloat4x4(&projectionTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->projection, &projection, sizeof(XMFLOAT4X4));

	XMFLOAT3 worldPosition = GetWorldPosition();
	memcpy(&pMappedCamera->position, &worldPosition, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pCameraBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

void Camera::UpdateViewTransform() {
	// �÷��̾� ��ġ���� ���������� �ٶ󺸰� �������� ������� ����
	XMFLOAT3 worldPosition = GetWorldPosition();

	XMFLOAT3 lookAtWorld = Vector3::Add(worldPosition, GetWorldLookVector());
	viewTransform = Matrix4x4::LookAtLH(worldPosition, lookAtWorld, GetWorldUpVector());
}

void Camera::UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle) {
	projectionTransform = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(_fovAngle), _aspectRatio, _nearDistance, _farDistance);
}

void Camera::MoveFront(float distance, float _timeElapsed) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	float moveDistance = distance * _timeElapsed;
	if (0 < distance) // ������ ���� ���
		moveDistance = min(Vector3::Length(localPosition) - minDistance, moveDistance);
	else	// �ڷ� ���� ���
		moveDistance = min(maxDistance - Vector3::Length(localPosition), moveDistance);
	moveVector = Vector3::ScalarProduct(moveVector, moveDistance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void Camera::UpdateLocalTransform() {
	GameObject::UpdateLocalTransform();
	localTransform._41 = localPosition.x + localOffset.x;
	localTransform._42 = localPosition.y + localOffset.y;
	localTransform._43 = localPosition.z + localOffset.z;
}
void Camera::UpdateWorldTransform() {
	// 1��Ī�� ���
	//GameObject::UpdateWorldTransform();

	// 3��Ī�� ��� 
	if (auto pParentLock = pParent.lock()) {	// �θ� ���� ���
		worldTransform = Matrix4x4::Multiply(localTransform, Matrix4x4::MoveTransform(pParentLock->GetWorldPosition()));
	}
	else {	// �θ� ���� ���
		worldTransform = localTransform;
	}
	UpdateViewTransform();
}
void Camera::UpdateObject() {

	// ���� ����
	float bias = 5.0f;

	GameObject::UpdateObject();

	// ���� ���� ��ķ� �ٿ�� ���������� �����.
	BoundingFrustum b(XMLoadFloat4x4(&projectionTransform));

	// ���� ī�޶��� �̵�, ȸ���� �������ҿ� �����Ѵ�.
	b.Transform(*pBoundingFrustum, XMLoadFloat4x4(&worldTransform));

	// ���� ���Ϳ� ��ġ�� ������Ʈ���� �׷����� �ʴ� ���� ���� ���� ���� �������Һ��� ��¦ �� ũ�� �����.
	pBoundingFrustum->Origin = Vector3::Subtract(pBoundingFrustum->Origin, Vector3::ScalarProduct(GetWorldLookVector(), bias));
	pBoundingFrustum->Far += bias * 2.f;
}
