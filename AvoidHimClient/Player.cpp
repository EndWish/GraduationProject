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

	// �ʱ� ��ġ ����
	SetLocalPosition(XMFLOAT3(0, 0, 0));

	name = "�÷��̾�";

	UpdateObject();

}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}

shared_ptr<GameObject> Player::GetRevObj() const {

	return pChildren[1];
}

void Player::SetCamera(shared_ptr<Camera> _pCamera) {
	// ������ ���� �� ������Ʈ�� �÷��̾��� �ڽ����� ���� �� ī�޶� �����Ѵ�.
	shared_ptr<GameObject> revObj = make_shared<GameObject>();
	revObj->SetChild(_pCamera);

	SetChild(revObj);
	pCamera = _pCamera;
}

void Player::Animate(double _timeElapsed) {

	if (pChildren[0]) pChildren[0]->Animate(_timeElapsed);

}

void Player::UpdateObject() {
	GameObject::UpdateObject();
	if(pCamera.lock()) pCamera.lock()->UpdateObject();
}
