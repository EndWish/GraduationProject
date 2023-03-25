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
	baseSpeed = 5.0f;

	mp = 100.0f;
	mpTick = 1.f;
	slowRate = 0;
	slowTime = 0;
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
	pCamera->SetLocalPosition(XMFLOAT3(0.f, 0.0f, -1.5f));	// ***** y���� �ٲܰ�� Camera�� GetCurrentDistance�� �ٲ�����Ѵ�.
	pCamera->SetLocalOffset(XMFLOAT3(0, 1, 0));
	pCamera->SetMaxDistance(1.5f);
	pCamera->SetMinDistance(0.3f);
	SetChild(pCamera);
	pCamera->UpdateObject();

	pFootStepSound = gameFramework.GetSoundManager().LoadFile("step");

	SetBoundingBox(BoundingOrientedBox(	XMFLOAT3(0, 0.88, 0),
										XMFLOAT3(0.317352414, 0.88, 0.274967313),
										XMFLOAT4(0, 0, 0, 1)));

	auto pSkinnedChild = dynamic_pointer_cast<SkinnedGameObject>(pChildren[0]);
	if (pSkinnedChild) {
		wpAniController = pSkinnedChild->GetAniController();
	}

	UpdateObject();

}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

void Player::Animate(char _collideCheck, float _timeElapsed) {
	if (auto pAniController = wpAniController.lock()) {
		pAniController->AddTime(_timeElapsed);
	}

	XMFLOAT3 prevPosition = GetWorldPosition();
	XMFLOAT3 position;
	if (speed > baseSpeed) {
		// �ӵ� �ʱ�ȭ
		speed = baseSpeed;
	}
	else {
		// ���¹̳� ȸ��
		mp += mpTick * _timeElapsed;
		mp = min(100.0f, mp);
	}

	sendMovePacketTime += (float)_timeElapsed;
	// y�������� �浹���� ���� ���
	if (_collideCheck & (1<<0)) {
		MoveUp(velocity.y, (float)_timeElapsed);
		landed = false;
		pFloor = nullptr;
	}
	else {
		// �ٴڰ� �浹�� y���� �ӵ��� �ʱ�ȭ ���ش�.
		velocity.y = 0.f;
		landed = true;
	}
	if (!(_collideCheck & (1<<1))) {
		MoveFront(velocity.z);
		Move(knockBack);
		// �ε��� ��� ��ü�� �ٶ󺸴� ������ �ݴ� �������� �о��ش�.
	}
	Rotate(rotation);
	


	// �����ӿ� ���� �̵� �� ȸ������ �ʱ�ȭ���ش�.
	velocity.x = 0;
	velocity.z = 0;
	knockBack = XMFLOAT3();
	rotation = Vector4::QuaternionIdentity();

	if (pChildren[0]) pChildren[0]->Animate(_timeElapsed);

	// �ӵ��� ������Ʈ ���ش�.
	UpdateRigidBody(_timeElapsed);

	// ��������� ������Ʈ ���ش�.
	GameObject::UpdateObject();
	pFootStepSound->SetPosition(GetWorldPosition());

	position = GetWorldPosition();
	if (!(_collideCheck & 1)) {
		moveDistance += Vector3::Length(Vector3::Subtract(prevPosition, position));
	}


	if (moveDistance > 1.0f) {
		pFootStepSound->Play();
		moveDistance = 0.f;
	}

	if (slowTime > 0) {
		slowTime -= _timeElapsed;
	}
	else {
		slowRate = 0.f;
	}

	// �������� �����θ�ŭ �������ش�.
	if (sendMovePacketTime >= SERVER_PERIOD) {
		sendMovePacketTime -= SERVER_PERIOD;
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

void Player::Dash(float _timeElapsed) {
	float costPerSec = 10.0f;

	if (mp > (costPerSec * _timeElapsed))
	{
		speed = baseSpeed * 1.5f;


		mp -= costPerSec * _timeElapsed;
	}
}


void Player::SetSlowRate(float _slowRate) {
	slowRate = _slowRate;
}

void Player::SetSlowTime(float _slowTime) {
	slowTime = _slowTime;
}

shared_ptr<AnimationController> Player::GetAniController() {
	return wpAniController.lock();
}


///////////////////////////////////////////////////////////////////////////////
/// Student

Student::Student() {
	hp = 100.0f;
	imprisoned = false;
	item = ObjectType::none;
}

Student::~Student() {

}

void Student::LeftClick() {
	// ������ ���
	// ����� �ٸ������� ó���Ѵ�.


	shared_ptr<Image2D> pUI;
	if (item == ObjectType::prisonKeyItem || item == ObjectType::none) return;

	else if (item == ObjectType::medicalKitItem) {
		pUI = Scene::GetUI("2DUI_medicalKit");
		// ü�� 50% ȸ��
		CS_USE_ITEM packet;
		packet.cid = cid;
		packet.itemType = item;
		packet.playerObjectID = myObjectID;
		SendFixedPacket(packet);
	}
	else if (item == ObjectType::energyDrinkItem) {
		pUI = Scene::GetUI("2DUI_energyDrink");
		// ���¹̳� 100% ȸ��
		SetMP(100.0f);
	}
	else if (item == ObjectType::trapItem) {
		// ���߿����� ����� �� ����.
		if (!landed) return;
		pUI = Scene::GetUI("2DUI_trap");
		CS_USE_ITEM packet;
		packet.cid = cid;
		packet.itemType = item;
		packet.playerObjectID = myObjectID;
		packet.usePosition = GetWorldPosition();
		SendFixedPacket(packet);

	}
	Scene::GetUI("2DUI_leftSkill")->SetEnable(true);
	Scene::GetUI("2DUI_leftSkill")->SetDark(true);
	if (pUI) pUI->SetEnable(false);
	item = ObjectType::none;
}

void Student::RightClick() {
	// ���� ��ų ���
	return;
	//pTexts["rightCoolTime"]->SetEnable(true);
	Scene::GetUI("2DUI_rightSkill")->SetEnable(true);
	Scene::GetUI("2DUI_rightSkill")->SetDark(true);
}

///////////////////////////////////////////////////////////////////////////////
/// Professor

Professor::Professor() {
	sabotageCoolTime = 0.0f;

	attackRemainCoolTime.fill(0.f);
	attackMaxCoolTime[(size_t)AttackType::swingAttack] = 1.0f;
	attackMaxCoolTime[(size_t)AttackType::throwAttack] = 2.0f;

	speed *= 1.2f;
	baseSpeed *= 1.2f;
}

Professor::~Professor() {
}

void Professor::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	Player::Create(_ObjectName, _pDevice, _pCommandList);
	wpHandObject = FindFrame("Bip001 R Hand");
}

void Professor::SetCoolTime(AttackType _type, float _coolTime) {
	attackRemainCoolTime[(size_t)_type] = _coolTime;
}

float Professor::GetSabotageCoolTime() const {
	return sabotageCoolTime;
}

void Professor::SetSabotageCoolTime(float _sabotageCoolTime) {
	sabotageCoolTime = _sabotageCoolTime;
}

void Professor::Reload(AttackType _type) {
	SetCoolTime(_type, attackMaxCoolTime[(size_t)_type]);
}

float Professor::GetCoolTime(AttackType _type) const {
	return attackRemainCoolTime[(size_t)_type];
}

void Professor::Animate(char _collideCheck, float _timeElapsed) {
	Player::Animate(_collideCheck, _timeElapsed);

	sabotageCoolTime -= _timeElapsed;


	for (auto& coolTime : attackRemainCoolTime) {
		if (coolTime >= 0.f) coolTime -= _timeElapsed;
	}



}

void Professor::LeftClick() {
	// �ֵθ��� ����
	if (GetCoolTime(AttackType::swingAttack) <= 0.f) {
		CS_ATTACK sendPacket;
		sendPacket.attackType = AttackType::swingAttack;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		Reload(AttackType::swingAttack);
		SendFixedPacket(sendPacket);
	}

	Scene::GetText("leftCoolTime")->SetEnable(true);
	Scene::GetUI("2DUI_swingAttack")->SetDark(true);

	// �������ݿ� ���� dark�� set
}

void Professor::RightClick() {
	if (GetCoolTime(AttackType::throwAttack) <= 0.f) {
		CS_ATTACK sendPacket;
		sendPacket.attackType = AttackType::throwAttack;
		sendPacket.cid = cid;
		sendPacket.playerObjectID = myObjectID;
		// ������ �ʾ��� ��� �̰����� ��� ��Ÿ���� ���� ������� ����ؼ� ��Ŷ�� �����ϰ� �ȴ�.
		// ���� �������� ��Ŷ�� �޾� ������ ������ ������ �� �ٽ� ��Ÿ���� �����Ѵ�.
		Reload(AttackType::throwAttack);
		SendFixedPacket(sendPacket);
	}

	Scene::GetText("rightCoolTime")->SetEnable(true);
	Scene::GetUI("2DUI_throwAttack")->SetDark(true);
	// ���Ÿ� ���ݿ� ���� dark�� set
}

shared_ptr<GameObject> Professor::GetHandObject() {
	return wpHandObject.lock();
}

