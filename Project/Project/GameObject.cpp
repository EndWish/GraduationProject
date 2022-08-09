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

XMFLOAT4X4 GameObject::GetFrontMoveMatrix(float _distance) const {
	// 이동할 벡터 = look단위 벡터에 거리를 곱한값
	XMFLOAT3 moveVector = Vector3::ScalarProduct(GetLookUnitVector(), _distance);
	XMFLOAT4X4 result = Matrix4x4::Identity();
	result._41 = moveVector.x;
	result._42 = moveVector.y;
	result._43 = moveVector.z;
	return result;
}
XMFLOAT4X4 GameObject::GetRightMoveMatrix(float _distance) const {
	// 이동할 벡터 = look단위 벡터에 거리를 곱한값
	XMFLOAT3 moveVector = Vector3::ScalarProduct(GetRightUnitVector(), _distance);
	XMFLOAT4X4 result = Matrix4x4::Identity();
	result._41 = moveVector.x;
	result._42 = moveVector.y;
	result._43 = moveVector.z;
	return result;
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

void GameObject::SetEachPosition(XMFLOAT3& _position) {
	eachTransform._41 = _position.x;
	eachTransform._42 = _position.y;
	eachTransform._43 = _position.z;
	UpdateWorldTransform();
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

void GameObject::ApplyTransform(XMFLOAT4X4& _transform) {
	eachTransform = Matrix4x4::Multiply(eachTransform, _transform);
	UpdateWorldTransform();
}

void GameObject::Animate(double _timeElapsed) {
	cout << format("GameObject({}) : 애니메이션 실행\n");
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	cout << format("GameObject({}) : 렌더 실행\n");
	for (const auto& pChild : pChildren) {
		pChild->Render(_pCommandList);
	}
}
