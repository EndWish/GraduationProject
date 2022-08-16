#include "stdafx.h"
#include "Room.h"


Room::Room() {

}

Room::~Room() {

}

void Room::AnimateObjects(double _timeElapsed) {
	for (const auto& pItem : pItems) {
		pItem->Animate(_timeElapsed);
	}

	for (const auto& pEffect : pEffects) {
		pEffect->Animate(_timeElapsed);
	}

	for (const auto& pPlayerAttack : pPlayerAttacks) {
		pPlayerAttack->Animate(_timeElapsed);
	}

	for (const auto& pEnemyAttack : pEnemyAttacks) {
		pEnemyAttack->Animate(_timeElapsed);
	}

	for (const auto& pObstacle : pObstacles) {
		pObstacle->Animate(_timeElapsed);
	}


}

int Room::GetID() const {
	return id;
}

string Room::GetType() const {
	return type;
}

const BoundingOrientedBox& Room::GetBoundingBox() const {
	return boundingBox;
}

const vector<weak_ptr<Room>>& Room::GetSideRooms() const {
	return pSideRooms;
}


void Room::SetType(string _type) {
	type = _type;
}

void Room::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	for (const auto& pItem : pItems) {
		pItem->Render(_pCommandList);
	}

	for (const auto& pEffect : pEffects) {
		pEffect->Render(_pCommandList);
	}

	for (const auto& pPlayerAttack : pPlayerAttacks) {
		pPlayerAttack->Render(_pCommandList);
	}

	for (const auto& pEnemyAttack : pEnemyAttacks) {
		pEnemyAttack->Render(_pCommandList);
	}

	for (const auto& pObstacle : pObstacles) {
		pObstacle->Render(_pCommandList);
	}
}