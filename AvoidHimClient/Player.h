#pragma once
#include "GameObject.h"
#include "Camera.h"
//#include "Status.h" 
class Player : public GameObject {
//class Player : public GameObject {

protected:
	// 0~100
	float moveDistance;
	float mp;
	// 자연 마나 회복량
	float mpTick;

	bool landed;
	float mass;
	float speed;
	float baseSpeed;

	XMFLOAT3 velocity;
	XMFLOAT4 rotation;

	XMFLOAT3 knockBack;
	shared_ptr<Camera> pCamera;

	shared_ptr<Sound> pFootStepSound;

	shared_ptr<GameObject> pFloor;
	float sendMovePacketTime;

public:
	Player();
	virtual ~Player();

public:
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	virtual void Animate(char _collideCheck, float _timeElapsed);
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

	float GetMP() const { return mp; };
	void SetMP(float _mp) { mp = _mp; };
	void AddMP(float _mp) { mp += _mp; };

	float GetSpeed() const { return speed; };
	void SetSpeed(float _speed) { speed = _speed; };
	void Dash(float _timeElapsed);

	virtual void LeftClick() {};
	virtual void RightClick() {};
}; 

class Student : public Player {
protected:
	float hp;
	bool imprisoned;
	// 보유하고 있는 아이템을 type로만 저장
	ObjectType item;
public:
	Student();
	virtual ~Student();

	//virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	float GetHP() const { return hp; };
	void SetHP(float _hp) { hp = _hp; };
	void AddHP(float _hp) { hp = min(hp+_hp, 100.f); };

	bool GetImprisoned() const { return imprisoned; }
	void SetImprisoned(bool _imprisoned) { imprisoned = _imprisoned; }

	virtual void LeftClick();
	virtual void RightClick();

	ObjectType GetItem() const { return item; };
	void SetItem(ObjectType _objectType) { item = _objectType; };
};

class Professor : public Player {
protected:
	float sabotageCoolTime;

	array<float, (size_t)AttackType::num> attackRemainCoolTime;
	array<float, (size_t)AttackType::num> attackMaxCoolTime;
public:
	Professor();
	virtual ~Professor();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	float GetSabotageCoolTime() const;
	void SetSabotageCoolTime(float _sabotageCoolTime);
	void SetCoolTime(AttackType _type, float _coolTime);
	void Reload(AttackType _type);
	float GetCoolTime(AttackType _type) const;
	virtual void Animate(char _collideCheck, float _timeElapsed);
	
	virtual void LeftClick();
	virtual void RightClick();

};