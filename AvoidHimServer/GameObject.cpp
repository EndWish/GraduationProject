#include "stdafx.h"
#include "GameObject.h"

//////////////////////////////////////////


GameObject::GameObject() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT4(0, 0, 0, 1);
	scale = XMFLOAT3(1, 1, 1);
	isOOBBCover = false;
	modelBouningBox = BoundingOrientedBox();
	worldBouningBox = BoundingOrientedBox();
}
GameObject::~GameObject() {

}

void GameObject::Create() {

}
void GameObject::Create(const string& _ObjectName) {
	name = _ObjectName;
}

XMFLOAT3 GameObject::GetRightVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(1, 0, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(rotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetUpVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 1, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(rotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLookVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 0, 1);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(rotation));
	return rightVector;
}

void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	position = Vector3::Add(position, _moveVector);
}
void GameObject::MoveRight(float distance) {
	XMFLOAT3 moveVector = GetRightVector();	// RightVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	position = Vector3::Add(position, moveVector);
}
void GameObject::MoveUp(float distance) {
	XMFLOAT3 moveVector = GetUpVector();	// UpVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	position = Vector3::Add(position, moveVector);
}
void GameObject::MoveFront(float distance) {
	XMFLOAT3 moveVector = GetLookVector();	// LookVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	position = Vector3::Add(position, moveVector);
}

void GameObject::Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	rotation = Vector4::QuaternionMultiply(rotation, Vector4::QuaternionRotation(_axis, _angle * _timeElapsed));
}
void GameObject::Rotate(const XMFLOAT4& _quat) {
	rotation = Vector4::QuaternionMultiply(rotation, _quat);
}

void GameObject::UpdateWorldTransform() {
	worldTransform = Matrix4x4::Identity();
	// S

	worldTransform._11 = scale.x;
	worldTransform._22 = scale.y;
	worldTransform._33 = scale.z;
	// SxR
	worldTransform = Matrix4x4::Multiply(worldTransform, Matrix4x4::RotateQuaternion(rotation));
	// xT
	worldTransform._41 = position.x;
	worldTransform._42 = position.y;
	worldTransform._43 = position.z;

}

void GameObject::UpdateOOBB() {
	modelBouningBox.Transform(worldBouningBox, XMLoadFloat4x4(&worldTransform));
	XMStoreFloat4(&worldBouningBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&worldBouningBox.Orientation)));
}

bool GameObject::CheckCollision(GameObject* pGameObject) {
	return worldBouningBox.Intersects(pGameObject->worldBouningBox);
}

