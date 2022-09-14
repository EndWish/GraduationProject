#include "stdafx.h"
#include "Status.h"
#include "GameObject.h"

RigidBody::RigidBody() {
	vSpeed = 0;
	moveSpeed = 0.0f;
	moveVector = XMFLOAT3(0, 0, 0);
	rotateVector = XMFLOAT4(0, 0, 0, 1);
}


RigidBody::~RigidBody() {

}


void RigidBody::InitVector() {
	moveVector = XMFLOAT3(0, 0, 0);
	rotateVector = XMFLOAT4(0, 0, 0, 1);
}

void RigidBody::Jump(float _power) {
	vSpeed = _power;
}


void RigidBody::GravityAnimate(double _timeElapsed, GameObject& _gameObject) {
	vSpeed -= GRAVITY * _timeElapsed;
	XMFLOAT3 position = _gameObject.GetLocalPosition();
	
	float prevy = position.y;
	position.y += vSpeed * _timeElapsed;
	position.y = max(position.y, 0);
	moveVector.y = position.y - prevy;
	_gameObject.SetLocalPosition(position);
}

void RigidBody::RotateRigid(const XMFLOAT3& _axis, float _angle) {
	// 수정 요망. 현재는 rotateVector가 한 프레임 회전한 벡터의 반대 벡터를 가지고 있다.
	shared_ptr<GameObject> selfLock = self.lock();
	//rotateVector = Vector4::QuaternionMultiply(selfLock->GetLocalRotate(), Vector4::QuaternionRotation(_axis, _angle));
	selfLock->SetLocalRotation(Vector4::QuaternionMultiply(selfLock->GetLocalRotate(), Vector4::QuaternionRotation(_axis, _angle)));
}

void RigidBody::MoveFrontRigid(bool _isfront) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 frontVector = selfLock->GetLocalLookVector();
	frontVector = Vector3::Normalize(frontVector);	// 단위벡터로 바꾼후
	if(_isfront) frontVector = Vector3::ScalarProduct(frontVector, moveSpeed);	// 이동거리만큼 곱해준다.
	else frontVector = Vector3::ScalarProduct(frontVector, -moveSpeed);	// 이동거리만큼 곱해준다.
	moveVector = Vector3::Add(moveVector, frontVector);
}

void RigidBody::MoveRightRigid(bool _isright) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 rightVector = selfLock->GetLocalRightVector();
	rightVector = Vector3::Normalize(rightVector);	// 단위벡터로 바꾼후
	if(_isright) rightVector = Vector3::ScalarProduct(rightVector, moveSpeed);	// 이동거리만큼 곱해준다.
	else rightVector = Vector3::ScalarProduct(rightVector, -moveSpeed);	// 이동거리만큼 곱해준다.
	moveVector = Vector3::Add(moveVector, rightVector);
}

void RigidBody::ComeBack(const shared_ptr<GameObject>& _playerObj, const shared_ptr<GameObject>& _obsObj) {
	shared_ptr<GameObject> selfLock = self.lock();
	BoundingOrientedBox prevOOBB = _playerObj->GetBoundingBox();
	BoundingOrientedBox obsOOBB = _obsObj->GetBoundingBox();
	XMFLOAT3 move(0,0,0);
	prevOOBB.Center = Vector3::Subtract(prevOOBB.Center, moveVector);

	BoundingOrientedBox checkOOBB;

	if (moveVector.x != 0) {
		cout << "x충돌!!\n";
		checkOOBB = prevOOBB;
		checkOOBB.Center.x += moveVector.x;
		// x방향 때문에 충돌한 경우
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.x -= moveVector.x;
		}
	}
	if (moveVector.y != 0) {
		checkOOBB = prevOOBB;
		checkOOBB.Center.y += moveVector.y;
		cout << "y충돌!!\n";
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.y -= moveVector.y;
		}
	}

	if (moveVector.z != 0) {
		checkOOBB = prevOOBB;
		checkOOBB.Center.z += moveVector.z;
		cout << "z충돌!!\n";
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.z -= moveVector.z;
		}
	}
	selfLock->SetLocalPosition(Vector3::Add(selfLock->GetLocalPosition(), move));
	selfLock->UpdateObject();
}