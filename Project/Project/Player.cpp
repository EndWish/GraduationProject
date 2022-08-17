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
	newCamera->SetEachPosition(XMFLOAT3(0, 0, -10));
	pMesh = gameFramework.GetMeshManager().GetMesh("test", gameFramework.GetDevice(), gameFramework.GetCommandList());

}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {

}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}
