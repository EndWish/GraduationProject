#include "stdafx.h"
#include "GameObject.h"

//////////////////////////////////////////


GameObject::GameObject() {
	name = "unknown";
	type = ObjectType::none;
	id = 0;
	worldTransform = Matrix4x4::Identity();
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT4(0, 0, 0, 1);
	scale = XMFLOAT3(1, 1, 1);
	modelBouningBox = BoundingOrientedBox();
	worldBouningBox = BoundingOrientedBox();
}
GameObject::GameObject(const GameObject& _other) {
	name = _other.name;
	type = _other.type;
	id = _other.id;

	worldTransform = _other.worldTransform;
	position = _other.position;
	scale = _other.scale;
	rotation = _other.rotation;

	modelBouningBox = _other.modelBouningBox;
	worldBouningBox = _other.worldBouningBox;
}
GameObject& GameObject::operator=(const GameObject& _other) {
	name = _other.name;
	type = _other.type;
	id = _other.id;

	worldTransform = _other.worldTransform;
	position = _other.position;
	scale = _other.scale;
	rotation = _other.rotation;

	modelBouningBox = _other.modelBouningBox;
	worldBouningBox = _other.worldBouningBox;
	return *this;
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

