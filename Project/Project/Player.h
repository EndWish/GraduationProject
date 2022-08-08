#pragma once
#include "GameObject.h"

class Player : public GameObject {
private:
	// 플레이어가 죽은지를 판단
	bool isDead;


public:
	Player();
	~Player();

public:
	bool GetIsDead() const;
	void Animate(double _timeElapsed);
};

