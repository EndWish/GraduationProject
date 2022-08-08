#include "stdafx.h"
#include "Player.h"


Player::Player() {
	isDead = false;
}

Player::~Player() {

}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::Animate(double _timeElapsed) {

}