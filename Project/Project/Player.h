#pragma once
#include "GameObject.h"
#include "Camera.h"

class Player : public GameObject {
private:
	// �÷��̾ �������� �Ǵ�
	bool isDead;
	weak_ptr<Camera> pCamera;

public:
	Player();
	~Player();

public:
	void Create(string _ObjectName);
	bool GetIsDead() const;
	shared_ptr<Camera> GetCamera() const;

	void Animate(double _timeElapsed);
};

