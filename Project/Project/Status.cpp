#include "stdafx.h"
#include "Status.h"
#include "GameObject.h"


void RigidBody::GravityAnimate(double _timeElapsed, GameObject& _gameObject) {
	vSpeed -= GRAVITY * _timeElapsed;
	XMFLOAT3 position = _gameObject.GetLocalPosition();
	position.y += vSpeed * _timeElapsed;
	position.y = max(position.y, 0);
	_gameObject.SetLocalPosition(position);
}