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

	name = "플레이어";

	//shared_ptr<Camera> newCamera = make_shared<Camera>();
	//newCamera->Create(_pDevice, _pCommandList);

	//pCamera = newCamera;
	//SetChild(newCamera);

	// [임시]
	//newCamera->SetEachPosition(XMFLOAT3(0.5, 0.5, -10));

	pLight = make_shared<Light>(shared_from_this());
	// 클래스 상속 관계에서 포인터 형 변환시 dynamic_cast(런타임 이후 동작)을 사용하자
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
