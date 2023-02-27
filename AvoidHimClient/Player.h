#pragma once
#include "GameObject.h"
#include "Camera.h"
//#include "Status.h" 

class Player : public GameObject {
//class Player : public GameObject {
private:
	// 0~100
	float moveDistance;
	float hp;
	float mp;
	// 자연 마나 회복량
	float mpTick;

	bool landed;
	float mass;
	float speed;

	XMFLOAT3 velocity;
	XMFLOAT4 rotation;

	XMFLOAT3 knockBack;
	shared_ptr<Camera> pCamera;

	shared_ptr<Sound> pFootStepSound;

	shared_ptr<GameObject> pFloor;
	float sendMovePacketTime;
public:
	Player();
	~Player();

public:
	void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void Animate(char _collideCheck, float _timeElapsed);
	shared_ptr<Camera> GetCamera();

	void UpdateRigidBody(float _timeElapsed);

	void Jump(float _force);
	XMFLOAT3 GetVelocity() const { return velocity; };
	XMFLOAT4 GetRotation() const { return rotation; };

	void SetVelocity(XMFLOAT3 _vel) { velocity = _vel; };

	void SetFloor(shared_ptr<GameObject> _pFloor) { pFloor = _pFloor; };
	shared_ptr<GameObject> GetFloor() { return pFloor; };
	void SetKnockBack(XMFLOAT3 _knockBack) { knockBack = _knockBack; knockBack.y = 0; };
	XMFLOAT3 GetKnockBack() const { return knockBack; };
	
	void AddFrontVelocity(float _velocity);
	void AddRightVelocity(float _velocity);
	void AddRotation(XMFLOAT4& _rotation);
	
	virtual void RotateMoveHorizontal(XMFLOAT3 _dir, float _angularSpeed, float _moveSpeed);
	float GetHP() const { return hp; };
	float GetMP() const { return mp; };
	
	void SetHP(float _hp) { hp = _hp; };
	void SetMP(float _mp) { mp = _mp; };

	void AddHP(float _hp) { hp += _hp; };
	void AddMP(float _mp) { mp += _mp; };

	float GetSpeed() const { return speed; };
	void Dash(float _timeElapsed);

};
