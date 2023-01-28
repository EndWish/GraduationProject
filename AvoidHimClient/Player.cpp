#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"



Player::Player() {

}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);

	GameFramework& gameFramework = GameFramework::Instance();

	// 초기 위치 설정
	SetLocalPosition(XMFLOAT3(0, 0, 0));
	name = "플레이어";

	//카메라 설정
	pCamera = make_shared<Camera>();
	pCamera->Create(_pDevice, _pCommandList);
	pCamera->SetLocalPosition(XMFLOAT3(0.f, 0.f, -1.5f));
	SetChild(pCamera);
	pCamera->UpdateObject();

	UpdateObject();

}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

void Player::Animate(double _timeElapsed) {

	if (pChildren[0]) pChildren[0]->Animate(_timeElapsed);

}

shared_ptr<Camera> Player::GetCamera() {
	return pCamera;
}

