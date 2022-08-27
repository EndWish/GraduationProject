#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"

Player::Player() {
	isDead = false;
	rigid.vSpeed = 0;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);
	SetLocalPosition(XMFLOAT3(0, 5, 0));
	GameFramework& gameFramework = GameFramework::Instance();

	name = "�÷��̾�";

	//shared_ptr<Camera> newCamera = make_shared<Camera>();
	//newCamera->Create(_pDevice, _pCommandList);

	//pCamera = newCamera;
	//SetChild(newCamera);

	// [�ӽ�]
	//newCamera->SetEachPosition(XMFLOAT3(0.5, 0.5, -10));

	pLight = make_shared<Light>(shared_from_this());
	// Ŭ���� ��� ���迡�� ������ �� ��ȯ�� dynamic_cast(��Ÿ�� ���� ����)�� �������
	auto playScene = dynamic_pointer_cast<PlayScene>(gameFramework.GetCurrentScene());
	playScene->AddLight(pLight);

	UpdateObject();
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {
	rigid.GravityAnimate(_timeElapsed, *this);
}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}
