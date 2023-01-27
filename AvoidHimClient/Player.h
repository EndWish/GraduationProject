#pragma once
#include "GameObject.h"
#include "Camera.h"
//#include "Status.h" 


//class Player : public GameObject, public RigidBody {
class Player : public GameObject {
private:
	weak_ptr<Camera> pCamera;

public:
	Player();
	~Player();

public:
	void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	shared_ptr<Camera> GetCamera() const;
	shared_ptr<GameObject> GetRevObj() const;
	void SetCamera(shared_ptr<Camera> _pCamera);
	void Animate(double _timeElapsed);
	virtual void UpdateObject();

};
