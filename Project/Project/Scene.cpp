#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

Scene::Scene() {
	
}

Scene::~Scene() {

}

///////////////////////////////////////////////////////////////////////////////
/// PlayScene
PlayScene::PlayScene(int _stageNum) {

	// ù ������������ �÷��̾� ����
	if (_stageNum == 1) {
		pPlayer[0] = make_shared<Player>();
		pPlayer[0]->Create();
		pPlayer[1] = make_shared<Player>();
		pPlayer[1]->Create();
	}
	// �� ����
	string fileName = "Stage";
	fileName += (to_string(_stageNum) + ".bin");

	LoadRoomsForFile(fileName);

	// ���� �� �÷��̾ �ִ� ���� ù������ ����
	//pNowRoom[0] = pRooms[0];
	//pNowRoom[1] = pRooms[0];

}

PlayScene::~PlayScene() {

}

void PlayScene::FrameAdvance(double _timeElapsed) {
	GameFramework& gameFramework = GameFramework::Instance();

	// �浹�˻縦 ������ ����� üũ.
	AnimateObjects(_timeElapsed);

	

	//if (pNowRoom[0]->GetType() == "Enemy" && pNowRoom[0]->GetID() == pNowRoom[1]->GetID()) {
		// �� ���� ����
		// Ŭ������� �ٸ������� �̵� �Ұ�
	//}
	
}

void PlayScene::AnimateObjects(double _timeElapsed) {
	// �÷��̾ ����ִ� ��� �ִϸ��̼��� ����
	if (!pPlayer[0]->GetIsDead()) {
		pPlayer[0]->Animate(_timeElapsed);
	}

	if (!pPlayer[1]->GetIsDead()) {
		pPlayer[1]->Animate(_timeElapsed);
	}

	// �� ���� ��鿡 ���� �ִϸ��̼��� ����
	for (const auto& room : pRooms) {
		room->AnimateObjects(_timeElapsed);
	}
}
void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// �����ӿ�ũ���� ������ ���� ��Ʈ�ñ״�ó�� set
	shared_ptr<Camera> pP1Camera = pPlayer[0]->GetCamera();
	pP1Camera->SetViewPortAndScissorRect();
	pP1Camera->UpdateShaderVariable();

	pPlayer[0]->Render(_pCommandList);

	// �� �������� ������ �ɷ����Ƿ� 
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}
	
}

void PlayScene::LoadRoomsForFile(string _fileName) {

	// �������� �� ���� ������ŭ �� �迭�� ���� �Ҵ�
	// pRooms.resize(�� ũ��);

	// �������� ���� ���� �� ������ �̸��� ����

	// ���� ������ǥ ���� ��ǥ(Center), ���� ����(Extent)�� �ҷ���

	// �� �� ������Ʈ���� �ε�

	// ��� ������ ����� ����
}

void PlayScene::CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum) {

	// ���� ������ �����ߴ� ��� ���� �浹üũ
	if (pNowRoom[_playerNum]->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {	// �浹�� ���
	}
	// ���� �濡�� �������ִ� ��� �浹üũ
	else {
		for (const auto& room : pNowRoom[_playerNum]->GetSideRooms()) {
			if (room.lock()->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {
				pNowRoom[_playerNum] = room.lock();
			}
		}
	}
}