#pragma once
#include "GameObject.h"
#include "Camera.h"

class Player : public GameObject {
private:
	// 플레이어가 죽은지를 판단
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

