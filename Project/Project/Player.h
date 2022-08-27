#pragma once
#include "GameObject.h"
#include "Status.h"
#include "Camera.h"

class Player : public GameObject {
private:
	// 플레이어가 죽은지를 판단
	bool isDead;
	RigidBody rigid;
	weak_ptr<Camera> pCamera;


public:
	Player();
	~Player();

public:
	void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	bool GetIsDead() const;
	RigidBody& GetRigid() { return rigid; };
	shared_ptr<Camera> GetCamera() const;

	void Animate(double _timeElapsed);
};

