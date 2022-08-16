#include "stdafx.h"
#include "Player.h"


Player::Player() {

}

Player::~Player() {

}

void Player::Create() {
	isDead = false;

	shared_ptr<Camera> newCamera = make_shared<Camera>();
	pCamera = newCamera;
	SetChild(newCamera);
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {

}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}
