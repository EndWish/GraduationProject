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

void GameObject::Animate(float _timeElapsed) {

}

///////////////////////////////////////////////////////////////////////////////
/// Player
Player::Player() : GameObject() {
	imprisoned = false;
}
Player::Player(const Player& _other) : GameObject(_other) {
	imprisoned = _other.imprisoned;
}
Player& Player::operator=(const Player& _other) {
	GameObject::operator=(_other);
	imprisoned = _other.imprisoned;
	return *this;
}
Player::~Player() {
}



///////////////////////////////////////////////////////////////////////////////
/// Door

Door::Door() : GameObject() {
	isExitDoor = false;
	isOpen = false;
}
Door::Door(const Door& _other) : GameObject(_other) {
	isExitDoor = _other.isExitDoor;
	isOpen = _other.isOpen;
}
Door& Door::operator=(const Door& _other) {
	GameObject::operator=(_other);
	isExitDoor = _other.isExitDoor;
	isOpen = _other.isOpen;
	return *this;
}
Door::~Door() {

}

///////////////////////////////////////////////////////////////////////////////
/// WaterDispenser

WaterDispenser::WaterDispenser() : GameObject() {
	coolTime = 0.f;
}
WaterDispenser::WaterDispenser(const WaterDispenser& _other) : GameObject(_other) {
	coolTime = _other.coolTime;
}
WaterDispenser& WaterDispenser::operator=(const WaterDispenser& _other) {
	GameObject::operator=(_other);
	coolTime = _other.coolTime;
	return *this;
}
WaterDispenser::~WaterDispenser() {

}
void WaterDispenser::Animate(float _timeElapsed) {
	if(0 < coolTime)
		coolTime -= _timeElapsed;
}

///////////////////////////////////////////////////////////////////////////////
/// Lever

Lever::Lever() : GameObject() {
	power = false;
}
Lever::Lever(const Lever& _other) : GameObject(_other) {
	power = _other.power;
}
Lever& Lever::operator=(const Lever& _other) {
	GameObject::operator=(_other);
	power = _other.power;
	return *this;
}
Lever::~Lever() {

}

///////////////////////////////////////////////////////////////////////////////
/// Computer

Computer::Computer() : GameObject() {
	hackingRate = 0.f;
	use = false;
}
Computer::Computer(const Computer& _other) : GameObject(_other) {
	hackingRate = _other.hackingRate;
	use = _other.use;
}
Computer& Computer::operator=(const Computer& _other) {
	GameObject::operator=(_other);
	hackingRate = _other.hackingRate;
	use = _other.use;
	return *this;
}
Computer::~Computer() {

}


