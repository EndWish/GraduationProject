#include "stdafx.h"
#include "Player.h"


Player::Player() {
	isDead = false;

	shared_ptr<Camera> newCamera = make_shared<Camera>();
	pCamera = newCamera;
	SetChild(newCamera);
}

Player::~Player() {

}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {

}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}