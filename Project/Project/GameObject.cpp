#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	eachTransform = Matrix4x4::Identity();
	boundingBox = BoundingOrientedBox();
	isOOBBBCover = false;
}
GameObject::~GameObject() {

}

XMFLOAT3 GameObject::GetRightUnitVector() const {
	return Vector3::Normalize(eachTransform._11, eachTransform._12, eachTransform._13);
}
XMFLOAT3 GameObject::GetUpUnitVector() const {
	return Vector3::Normalize(eachTransform._21, eachTransform._22, eachTransform._23);
}
XMFLOAT3 GameObject::GetLookUnitVector() const {
	return Vector3::Normalize(eachTransform._31, eachTransform._32, eachTransform._33);
}
XMFLOAT3 GameObject::GetEachPosition() const {
	return XMFLOAT3(eachTransform._41, eachTransform._42, eachTransform._43);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

void GameObject::SetChild(const shared_ptr<GameObject> _pChild) {
	// 입양할 아이가, 부모가 있을 경우 부모로 부터 독립시킨다.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// 나의 자식으로 입양
	pChildren.push_back(_pChild);

	// 자식의 부모를 나로 지정
	_pChild->pParent = shared_from_this();
}

void GameObject::SetEachPosition(const XMFLOAT3& _position) {
	eachTransform._41 = _position.x;
	eachTransform._42 = _position.y;
	eachTransform._43 = _position.z;
	UpdateWorldTransform();
}

void GameObject::UpdateWorldTransform() {
	if (auto pParentLock = pParent.lock()) {	// 부모가 있을 경우
		worldTransform = Matrix4x4::Multiply(pParentLock->worldTransform, eachTransform);
	}
	else {	// 부모가 없을 경우
		worldTransform = eachTransform;
	}

	// 자식들도 worldTransform을 업데이트 시킨다.
	for (auto& pChild : pChildren) {
		pChild->UpdateWorldTransform();
	}
}

void GameObject::MoveFront(float _distance) {
	// look단위벡터에 _distance 를 곱한 값 만큼 이동한다.
	XMFLOAT3 destination = Vector3::Add(GetEachPosition(), GetLookUnitVector(), _distance);
	SetEachPosition(destination);
}

void GameObject::MoveRight(float _distance) {
	// look단위벡터에 _distance 를 곱한 값 만큼 이동한다.
	XMFLOAT3 destination = Vector3::Add(GetEachPosition(), GetRightUnitVector(), _distance);
	SetEachPosition(destination);
}

void GameObject::Animate(double _timeElapsed) {
	cout << format("GameObject({}) : 애니메이션 실행\n");
}
