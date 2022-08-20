#include "stdafx.h"
#include "GameObject.h"
#include "Light.h"

GameObject::GameObject() {

}
GameObject::~GameObject() {

}

void GameObject::Create() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	eachTransform = Matrix4x4::Identity();
	boundingBox = BoundingOrientedBox();
	isOOBBBCover = false;
}

XMFLOAT3 GameObject::GetEachRightVector() const {
	return Vector3::Normalize(eachTransform._11, eachTransform._12, eachTransform._13);
}
XMFLOAT3 GameObject::GetEachUpVector() const {
	return Vector3::Normalize(eachTransform._21, eachTransform._22, eachTransform._23);
}
XMFLOAT3 GameObject::GetEachLookVector() const {
	return Vector3::Normalize(eachTransform._31, eachTransform._32, eachTransform._33);
}
XMFLOAT3 GameObject::GetEachPosition() const {
	return XMFLOAT3(eachTransform._41, eachTransform._42, eachTransform._43);
}

XMFLOAT3 GameObject::GetWorldRightVector() const {
	return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13);
}
XMFLOAT3 GameObject::GetWorldUpVector() const {
	return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);
}
XMFLOAT3 GameObject::GetWorldLookVector() const {
	return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33);
}
XMFLOAT3 GameObject::GetWorldPosition() const {
	return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43);
}

XMFLOAT4X4 GameObject::GetFrontMoveMatrix(float _distance) const {
	// �̵��� ���� = look���� ���Ϳ� �Ÿ��� ���Ѱ�
	XMFLOAT3 moveVector = Vector3::ScalarProduct(GetEachLookVector(), _distance);
	XMFLOAT4X4 result = Matrix4x4::Identity();
	result._41 = moveVector.x;
	result._42 = moveVector.y;
	result._43 = moveVector.z;
	return result;
}
XMFLOAT4X4 GameObject::GetRightMoveMatrix(float _distance) const {
	// �̵��� ���� = look���� ���Ϳ� �Ÿ��� ���Ѱ�
	XMFLOAT3 moveVector = Vector3::ScalarProduct(GetEachRightVector(), _distance);
	XMFLOAT4X4 result = Matrix4x4::Identity();
	result._41 = moveVector.x;
	result._42 = moveVector.y;
	result._43 = moveVector.z;
	return result;
}
XMFLOAT4X4 GameObject::GetRotateMatrix(const XMFLOAT3& _axis, float _angle) const {
	return Matrix4x4::RotationAxis(_axis, _angle);
}
XMFLOAT4X4 GameObject::GetRotateMatrix(const XMFLOAT4& _quaternion) const {
	return Matrix4x4::RotateQuaternion(_quaternion);
}
XMFLOAT4X4 GameObject::GetRotateMatrix(float _pitch, float _yaw, float _roll) const {
	return Matrix4x4::RotatePitchYawRoll(_pitch, _yaw, _roll);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

void GameObject::SetEachPosition(const XMFLOAT3& _position) {
	eachTransform._41 = _position.x;
	eachTransform._42 = _position.y;
	eachTransform._43 = _position.z;
	UpdateWorldTransform();
}

void GameObject::SetChild(const shared_ptr<GameObject> _pChild) {
	// �Ծ��� ���̰�, �θ� ���� ��� �θ�� ���� ������Ų��.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// ���� �ڽ����� �Ծ�
	pChildren.push_back(_pChild);

	// �ڽ��� �θ� ���� ����
	_pChild->pParent = shared_from_this();
}

void GameObject::SetMesh(const shared_ptr<Mesh>& _pMesh) {
	pMesh = _pMesh;
}

void GameObject::UpdateWorldTransform() {
	if (auto pParentLock = pParent.lock()) {	// �θ� ���� ���
		worldTransform = Matrix4x4::Multiply(eachTransform, pParentLock->worldTransform);
	}
	else {	// �θ� ���� ���
		worldTransform = eachTransform;
	}

	// �ڽĵ鵵 worldTransform�� ������Ʈ ��Ų��.
	for (auto& pChild : pChildren) {
		pChild->UpdateWorldTransform();
	}
}

void GameObject::ApplyTransform(const XMFLOAT4X4& _transform) {
	eachTransform = Matrix4x4::Multiply(eachTransform, _transform);
	UpdateWorldTransform();
}

void GameObject::Animate(double _timeElapsed) {
	cout << format("GameObject({}) : �ִϸ��̼� ����\n");
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pMesh.lock()) {	// �޽��� ���� ��쿡�� �������� �Ѵ�.
		UpdateShaderVariable(_pCommandList);
		// ����� ���̴��� �׷��Ƚ� ������������ �����Ѵ� [�������]
		Mesh::GetShader()->PrepareRender(_pCommandList);
		pMesh.lock()->Render(_pCommandList);
		for (const auto& pChild : pChildren) {
			pChild->Render(_pCommandList);
		}

	}
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}


