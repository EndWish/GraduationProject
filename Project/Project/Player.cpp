#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"

Player::Player() {

}

Player::~Player() {

}

void Player::Create() {
	GameObject::Create();
	GameFramework& gameFramework = GameFramework::Instance();

	name = "플레이어";
	
	isDead = false;

	shared_ptr<Camera> newCamera = make_shared<Camera>();
	newCamera->Create();

	pCamera = newCamera;
	SetChild(newCamera);

	// [임시]
	newCamera->SetEachPosition(XMFLOAT3(0.5, 0.5, -10));
	pMesh = gameFramework.GetMeshManager().GetMesh("test", gameFramework.GetDevice(), gameFramework.GetCommandList());
	
	pLight = make_shared<Light>(shared_from_this());
	// 클래스 상속 관계에서 포인터 형 변환시 dynamic_cast(런타임 이후 동작)을 사용하자
	auto playScene = dynamic_pointer_cast<PlayScene>(gameFramework.GetCurrentScene());

	playScene->AddLight(pLight);
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {

}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}
