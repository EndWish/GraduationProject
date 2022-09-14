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
	// ���� ���. ����� rotateVector�� �� ������ ȸ���� ������ �ݴ� ���͸� ������ �ִ�.
	shared_ptr<GameObject> selfLock = self.lock();
	//rotateVector = Vector4::QuaternionMultiply(selfLock->GetLocalRotate(), Vector4::QuaternionRotation(_axis, _angle));
	selfLock->SetLocalRotation(Vector4::QuaternionMultiply(selfLock->GetLocalRotate(), Vector4::QuaternionRotation(_axis, _angle)));
}

void RigidBody::MoveFrontRigid(bool _isfront) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 frontVector = selfLock->GetLocalLookVector();
	frontVector = Vector3::Normalize(frontVector);	// �������ͷ� �ٲ���
	if(_isfront) frontVector = Vector3::ScalarProduct(frontVector, moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	else frontVector = Vector3::ScalarProduct(frontVector, -moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	moveVector = Vector3::Add(moveVector, frontVector);
}

void RigidBody::MoveRightRigid(bool _isright) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 rightVector = selfLock->GetLocalRightVector();
	rightVector = Vector3::Normalize(rightVector);	// �������ͷ� �ٲ���
	if(_isright) rightVector = Vector3::ScalarProduct(rightVector, moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	else rightVector = Vector3::ScalarProduct(rightVector, -moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
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
		cout << "x�浹!!\n";
		checkOOBB = prevOOBB;
		checkOOBB.Center.x += moveVector.x;
		// x���� ������ �浹�� ���
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.x -= moveVector.x;
		}
	}
	if (moveVector.y != 0) {
		checkOOBB = prevOOBB;
		checkOOBB.Center.y += moveVector.y;
		cout << "y�浹!!\n";
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.y -= moveVector.y;
		}
	}

	if (moveVector.z != 0) {
		checkOOBB = prevOOBB;
		checkOOBB.Center.z += moveVector.z;
		cout << "z�浹!!\n";
		if (checkOOBB.Intersects(_obsObj->GetBoundingBox())) {
			move.z -= moveVector.z;
		}
	}
	selfLock->SetLocalPosition(Vector3::Add(selfLock->GetLocalPosition(), move));
	selfLock->UpdateObject();
}