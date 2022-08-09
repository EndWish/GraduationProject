#pragma once
#include "GameObject.h"

class Player : public GameObject {
private:
	// �÷��̾ �������� �Ǵ�
	bool isDead;


public:
	Player();
	~Player();

public:
	bool GetIsDead() const;
	void Animate(double _timeElapsed);
};

