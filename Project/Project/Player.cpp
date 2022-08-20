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

	name = "�÷��̾�";
	
	isDead = false;

	shared_ptr<Camera> newCamera = make_shared<Camera>();
	newCamera->Create();

	pCamera = newCamera;
	SetChild(newCamera);

	// [�ӽ�]
	newCamera->SetEachPosition(XMFLOAT3(0.5, 0.5, -10));
	pMesh = gameFramework.GetMeshManager().GetMesh("test", gameFramework.GetDevice(), gameFramework.GetCommandList());
	
	pLight = make_shared<Light>(shared_from_this());
	// Ŭ���� ��� ���迡�� ������ �� ��ȯ�� dynamic_cast(��Ÿ�� ���� ����)�� �������
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
