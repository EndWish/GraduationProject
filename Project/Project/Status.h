#pragma once

class GameObject;

struct RigidBody {
	float vSpeed;

	void GravityAnimate(double _timeElapsed, GameObject& _gameObject);
};
