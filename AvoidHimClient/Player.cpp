#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"



Player::Player() {
	landed = false;
	mass = 100.0f;
	velocity = XMFLOAT3();
	knockBack = XMFLOAT3();
	rotation = Vector4::QuaternionIdentity();
	sendMovePacketTime = 0.f;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);

	GameFramework& gameFramework = GameFramework::Instance();

	// �ʱ� ��ġ ����
	SetLocalPosition(XMFLOAT3(0, 0, 0));
	name = "�÷��̾�";

	//ī�޶� ����
	pCamera = make_shared<Camera>();
	pCamera->Create(_pDevice, _pCommandList);
	pCamera->SetLocalPosition(XMFLOAT3(0.f, 1.0f, -1.5f));
	SetChild(pCamera);
	pCamera->UpdateObject();

	UpdateObject();

}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

void Player::Animate(char _collideCheck, double _timeElapsed) {

	sendMovePacketTime += (float)_timeElapsed;
	// y�������� �浹���� ���� ���
	if (_collideCheck & 1) {
		MoveUp(velocity.y, (float)_timeElapsed);
		landed = false;
		pFloor = nullptr;
	}
	else {
		// �ٴڰ� �浹�� y���� �ӵ��� �ʱ�ȭ ���ش�.
		velocity.y = 0.f;
		landed = true;
	}

	if (_collideCheck & 2) {
		MoveFront(velocity.z);
	}

	// �ε��� ��� ��ü�� �ٶ󺸴� ������ �ݴ� �������� �о��ش�.
	Rotate(rotation);
	if (!(_collideCheck & 4)) {
		Move(knockBack);
	}

	// �����ӿ� ���� �̵� �� ȸ������ �ʱ�ȭ���ش�.
	velocity.x = 0;
	velocity.z = 0;
	rotation = Vector4::QuaternionIdentity();

	if (pChildren[0]) pChildren[0]->Animate(_timeElapsed);

	// �ӵ��� ������Ʈ ���ش�.
	UpdateRigidBody(_timeElapsed);


	// ��������� ������Ʈ ���ش�.
	GameObject::UpdateObject();

	// �������� �����θ�ŭ �������ش�.

	if (sendMovePacketTime > SEND_PACKET_PERIOD) {
		sendMovePacketTime = 0.f;
		CS_PLAYER_INFO packet;
		packet.cid = cid;
		packet.position = localPosition;
		packet.rotation = localRotation;
		packet.scale = localScale;
		packet.objectID = id;
		if (packet.cid == 0)
			cout << name << "\n";
		SendFixedPacket(packet);
	}

}

shared_ptr<Camera> Player::GetCamera() {
	return pCamera;
}


void Player::UpdateRigidBody(double _timeElapsed) {

	if (!landed) {
		velocity.y -= GRAVITY * _timeElapsed;
		//velocity.y = max(MAX_FALLSPEED, velocity.y);
	}
}

void Player::Jump(float _force) {
	if (landed) {
		landed = false;
		velocity.y += _force / mass;
	}
}

void Player::AddFrontVelocity(float _velocity) {
	velocity.z += _velocity;
}

void Player::AddRightVelocity(float _velocity) {
	velocity.x += _velocity;
}

void Player::AddRotation(XMFLOAT4& _rotation) {
	rotation = Vector4::QuaternionMultiply(rotation, _rotation);
}

void Player::RotateMoveHorizontal(XMFLOAT3 _dir, float _angularSpeed, float _moveSpeed) {
	// �躤�Ϳ� Ÿ�ٺ��͸� xz��鿡 �����Ѵ�.
	_dir.y = 0;
	XMFLOAT3 origin = GetLocalLookVector();
	origin.y = 0;
	XMFLOAT3 axis = Vector3::CrossProduct(origin, _dir);
	float minAngle = Vector3::Angle(origin, _dir, false);
	if (abs(axis.y) <= numeric_limits<float>::epsilon()) {	// ������ �Ұ����� ��� (�� ���Ͱ� ������ ���)
		axis = XMFLOAT3(0, 1, 0);
	}
	XMFLOAT4 rot = Vector4::QuaternionRotation(axis, min(_angularSpeed, minAngle));
	AddRotation(rot);
	AddFrontVelocity(_moveSpeed);
}