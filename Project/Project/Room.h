#pragma once
#include "GameObject.h"

class Room {
public:
	
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

public:
	int GetID() const;
	string GetType() const;
	vector<weak_ptr<Room>>& GetPSideRooms();
	const BoundingOrientedBox& GetBoundingBox() const;
	const vector<weak_ptr<Room>>& GetSideRooms() const;

	void SetType(string _type);

	void AnimateObjects(double _timeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	vector<int> LoadRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<GameObject> LoadObjectFromRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

