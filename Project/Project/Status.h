#pragma once

class GameObject;

// RigidBody 멤버 자체를 private로 하면 되므로 struct로 함
class RigidBody {
protected:
	float vSpeed;
	float moveSpeed;
	XMFLOAT3 moveVector;
	XMFLOAT4 rotateVector;

	weak_ptr<GameObject> self;
public:
	RigidBody();
	~RigidBody();

	void SetVSpeed(float _value);
	void SetRotateSpeed(float _value);
	void SetMoveSpeed(float _value);

	void InitVector();
	void Jump(float _power);


	void GravityAnimate(double _timeElapsed, GameObject& _gameObject);
	
	void RotateRigid(const XMFLOAT3& _axis, float _angle);
	void MoveFrontRigid(bool _isfront);
	void MoveRightRigid(bool _isright);

	void ComeBack(const shared_ptr<GameObject>& _playerObj, const shared_ptr<GameObject>& _obsObj);
};
