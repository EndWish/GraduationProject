#pragma once
#include "GameObject.h"

class Room {
private:
	// 방의 고유 번호
	int id;

	BoundingOrientedBox boundingBox;

	// 룸내에 인접한 다른 룸들
	vector<weak_ptr<Room>> pSideRooms;

	// 각종 오브젝트들
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

