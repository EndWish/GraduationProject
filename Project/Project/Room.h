#pragma once
#include "GameObject.h"

class Room {
private:
	// ���� ���� ��ȣ
	int id;

	BoundingOrientedBox boundingBox;

	// �볻�� ������ �ٸ� ���
	vector<weak_ptr<Room>> pSideRooms;

	// ���� ������Ʈ��
	vector<shared_ptr<GameObject>> pItems;
	vector<shared_ptr<GameObject>> pEffects;
	vector<shared_ptr<GameObject>> pPlayerAttacks;
	vector<shared_ptr<GameObject>> pEnemyAttacks;
	vector<shared_ptr<GameObject>> pObstacles;

public:
	Room();
	~Room();

public:
	void AnimateObjects(double _timeElapsed);
	const BoundingOrientedBox& GetBoundingBox() const;
	const vector<weak_ptr<Room>>& GetSideRooms() const;
};

