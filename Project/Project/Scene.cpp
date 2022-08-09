#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"

Scene::Scene() {
	
}

Scene::~Scene() {

}


PlayScene::PlayScene(int _stageNum) {

	// ù ������������ �÷��̾� ����
	if (_stageNum == 1) {
		//pPlayer[0] = make_shared<Player>();
		//pPlayer[1] = make_shared<Player>();
	}

	// �� ����
	string fileName = "Stage";
	fileName += (to_string(_stageNum) + ".bin");

	loadRoomsForFile(fileName);

	// ���� �� �÷��̾ �ִ� ���� ù������ ����
	pNowRoom[0] = pRooms[0];
	pNowRoom[1] = pRooms[0];

}

PlayScene::~PlayScene() {

}


void PlayScene::FrameAdvance(double _timeElapsed) {
	// �浹�˻縦 ������ ����� üũ.
	AnimateObjects(_timeElapsed);

	Room p1Room = checkCurrentRoom(pPlayer[0]->GetBoundingBox(), 0);
	Room p2Room = checkCurrentRoom(pPlayer[0]->GetBoundingBox(), 0);

	if (p1Room.GetType() == "Enemy" && p1Room == p2Room) {
		// �� ���� ����
		// Ŭ������� �ٸ������� �̵� �Ұ�
	}
	
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

	// �� �������� ������ �ɷ����Ƿ� 
	for (const auto& room : pRooms) {
		room->Render(_pCommandList);
	}

}


void PlayScene::loadRoomsForFile(string _fileName) {

	cout << _fileName << "�ε� !\n";
	// �������� �� ���� ������ŭ �� �迭�� ���� �Ҵ�
	// pRooms.resize(�� ũ��);

	// �������� ���� ���� �� ������ �̸��� ����

	// ���� ������ǥ ���� ��ǥ(Center), ���� ����(Extent)�� �ҷ���

	// �� �� ������Ʈ���� �ε�

	// ��� ������ ����� ����
}

const Room& PlayScene::checkCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum) {

	// ���� ������ �����ߴ� ��� ���� �浹üũ
	if (pNowRoom[_playerNum]->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {	// �浹�� ���
		return *pNowRoom[_playerNum];
	}
	// ���� �濡�� �������ִ� ��� �浹üũ
	else {
		for (const auto& room : pNowRoom[_playerNum]->GetSideRooms()) {
			if (room.lock()->GetBoundingBox().Contains(XMLoadFloat3(&_playerOOBB.Center)) != DISJOINT) {
				pNowRoom[_playerNum] = room.lock();
				return *room.lock();
			}
		}
	}
}