#pragma once
#include "GameObject.h"

class Room {
private:
	// ���� ���� ��ȣ
	int id;

	// ���� Ÿ��.
	string type;
	

	// �÷��̾ �� ���� �ִ��� Ȯ���ϱ� ����
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
	bool operator==(const Room& _other) const;

public:
	int GetID() const;
	string GetType() const;
	const BoundingOrientedBox& GetBoundingBox() const;
	const vector<weak_ptr<Room>>& GetSideRooms() const;

	void SetType(string _type);

	void AnimateObjects(double _timeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};

