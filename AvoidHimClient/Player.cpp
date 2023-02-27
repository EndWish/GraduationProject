#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"



Player::Player() {
	moveDistance = 0.f;
	landed = false;
	mass = 100.0f;
	velocity = XMFLOAT3();
	knockBack = XMFLOAT3();
	rotation = Vector4::QuaternionIdentity();
	sendMovePacketTime = 0.f;
	speed = 5.0f;

	hp = 100.0f;
	mp = 100.0f;
	mpTick = 1.f;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);

	GameFramework& gameFramework = GameFramework::Instance();

	// 초기 위치 설정
	SetLocalPosition(XMFLOAT3(0, 0, 0));
	name = "플레이어";

	//카메라 설정
	pCamera = make_shared<Camera>();
	pCamera->Create(_pDevice, _pCommandList);
	pCamera->SetLocalPosition(XMFLOAT3(0.f, 1.0f, -1.5f));	// ***** y축을 바꿀경우 Camera의 GetCurrentDistance도 바꿔줘야한다.
	pCamera->SetMaxDistance(1.5f);
	pCamera->SetMinDistance(0.3f);
	SetChild(pCamera);
	pCamera->UpdateObject();

	pFootStepSound = gameFramework.GetSoundManager().LoadFile("step");
	UpdateObject();

}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

void Player::Animate(char _collideCheck, float _timeElapsed) {


	XMFLOAT3 prevPosition = GetWorldPosition();
	XMFLOAT3 position;
	if (speed > 5.f) {
		// 속도 초기화
		speed = 5.f;
	}
	else {
		// 스태미너 회복
		mp += mpTick * _timeElapsed;
		mp = min(100.0f, mp);
	}

	sendMovePacketTime += (float)_timeElapsed;
	// y방향으로 충돌하지 않을 경우
	if (_collideCheck & 1) {
		MoveUp(velocity.y, (float)_timeElapsed);
		landed = false;
		pFloor = nullptr;
	}
	else {
		// 바닥과 충돌시 y방향 속도를 초기화 해준다.
		velocity.y = 0.f;
		landed = true;
	}

	if (_collideCheck & 2) {
		MoveFront(velocity.z);
	}

	// 부딪힐 경우 물체를 바라보는 방향의 반대 방향으로 밀어준다.
	Rotate(rotation);
	if (!(_collideCheck & 4)) {
		Move(knockBack);
	}


	// 프레임에 모인 이동 및 회전값을 초기화해준다.
	velocity.x = 0;
	velocity.z = 0;
	rotation = Vector4::QuaternionIdentity();

	if (pChildren[0]) pChildren[0]->Animate(_timeElapsed);

	// 속도를 업데이트 해준다.
	UpdateRigidBody(_timeElapsed);

	// 월드행렬을 업데이트 해준다.
	GameObject::UpdateObject();
	pFootStepSound->SetPosition(GetWorldPosition());

	position = GetWorldPosition();
	if (!(_collideCheck & 1)) {
		moveDistance += Vector3::Length(Vector3::Subtract(prevPosition, position));
	}

	if (moveDistance > 2.0f) {
		pFootStepSound->Play();
		moveDistance = 0.f;
	}

	// 서버에게 움직인만큼 전송해준다.
	cout << GetWorldPosition() << "\n";
	if (sendMovePacketTime > SERVER_PERIOD) {
		sendMovePacketTime = 0.f;
		CS_PLAYER_INFO packet;
		packet.cid = cid;
		packet.position = localPosition;
		packet.rotation = localRotation;
		packet.scale = localScale;
		packet.objectID = id;
		SendFixedPacket(packet);
	}

}

shared_ptr<Camera> Player::GetCamera() {
	return pCamera;
}


void Player::UpdateRigidBody(float _timeElapsed) {

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
	// 룩벡터와 타겟벡터를 xz평면에 투영한다.
	_dir.y = 0;
	XMFLOAT3 origin = GetLocalLookVector();
	origin.y = 0;
	XMFLOAT3 axis = Vector3::CrossProduct(origin, _dir);
	float minAngle = Vector3::Angle(origin, _dir, false);
	if (abs(axis.y) <= numeric_limits<float>::epsilon()) {	// 외적이 불가능한 경우 (두 벡터가 평행한 경우)
		axis = XMFLOAT3(0, 1, 0);
	}
	XMFLOAT4 rot = Vector4::QuaternionRotation(axis, min(_angularSpeed, minAngle));
	AddRotation(rot);
	AddFrontVelocity(_moveSpeed);
}

void Player::Dash(float _timeElapsed) {
	float costPerSec = 10.0f;

	if (mp > (costPerSec * _timeElapsed))
	{
		speed = 7.5f;
		mp -= costPerSec * _timeElapsed;
	}
}
