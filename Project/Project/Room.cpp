#include "stdafx.h"
#include "Room.h"
#include "Player.h"
#include "Door.h"
#include "Obstacle.h"

Room::Room() {
	id = 0;
	type = "none";
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

void Room::CheckCollision() {
	
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

vector<weak_ptr<Room>>& Room::GetSideRooms() {
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

vector<int> Room::LoadRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// roomID (UINT)
	_file.read((char*)&id, sizeof(UINT));

	// nEffect (UINT)
	UINT nEffect, nObstacle, nItem;

	_file.read((char*)&nEffect, sizeof(UINT));
	pEffects.reserve(nEffect);
	for (int i = 0; i < nEffect; ++i) {
		pEffects.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	_file.read((char*)&nObstacle, sizeof(UINT));
	pObstacles.reserve(nObstacle);
	for (int i = 0; i < nObstacle; ++i) {
		pObstacles.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	_file.read((char*)&nItem, sizeof(UINT));
	pItems.reserve(nItem);
	for (int i = 0; i < nItem; ++i) {
		pItems.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	vector<int> nextRooms;

	UINT nNextRoom, nextRoomID;
	_file.read((char*)&nNextRoom, sizeof(UINT));
	for (int i = 0; i < nNextRoom; ++i) {
		_file.read((char*)&nextRoomID, sizeof(UINT));
		nextRooms.push_back(nextRoomID);
	}
	return nextRooms;
}

shared_ptr<GameObject> Room::LoadObjectFromRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	string classType, fileName;
	ReadStringBinary(classType, _file);
	ReadStringBinary(fileName, _file);
	shared_ptr<GameObject> newObject;
	/*if (classType == "Door") {
		newObject = make_shared<Door>();
	}
	else if (classType == "Obstacle") {
		newObject = make_shared<Obstacle>();
	}
	else*/ {
		newObject = make_shared<GameObject>();
	}
	// GameObject에 담기는 기본 정보들 초기화, 오브젝트 매니저에서 불러옴
	newObject->Create(fileName, _pDevice, _pCommandList);

	XMFLOAT3 buffer;
	XMFLOAT4 bufferQ;
	// worldTransform (float3)
	_file.read((char*)&buffer, sizeof(XMFLOAT3));
	newObject->SetLocalPosition(buffer);

	// worldScale (float3)
	_file.read((char*)&buffer, sizeof(XMFLOAT3));
	newObject->SetLocalScale(buffer);

	// worldRotation (float4)
	_file.read((char*)&bufferQ, sizeof(XMFLOAT4));

	newObject->SetLocalRotation(bufferQ);

	newObject->UpdateLocalTransform();
	newObject->UpdateWorldTransform();

	newObject->UpdateOOBB();
	return newObject;
}