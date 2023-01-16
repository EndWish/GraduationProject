#pragma once

class GameObject;

class Sector {
	typedef unordered_map<UINT, GameObject*> Layer;

private:
	vector<Layer> pGameObjectLayers;

public:
	Sector();
	~Sector();

	// ���� ����
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pGameObject);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pGameObject);

	// ��ü ã��
	GameObject* FindObject(SectorLayer _sectorLayer, UINT _objectID);

};

class Zone {
	// GetAroundSectors(const XMINT3& _index)�� �Ҹ��� ���� _index�� ���� �Ҹ���� ������ ���� �� �ִ�.

private:
	XMFLOAT3 startPoint;	// ������ġ
	XMFLOAT3 size;			// �� ������ ũ��
	XMINT3 div;				// ������ ���� ����
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors;

public:
	// ������, �Ҹ���
	Zone();
	Zone(const XMFLOAT3& _size, const XMINT3& _div);
	~Zone();

	// get, set �Լ�
	vector<vector<vector<Sector>>>& GetAllSectors() { return sectors; }

	/// �Ϲ��Լ�
	// �ε����� ���� �� ���� Ȯ���ϴ� �Լ�
	bool IndexInside(const XMINT3& _index) {
		return (0 <= _index.x && _index.x < div.x) && (0 <= _index.y && _index.y < div.y) && (0 <= _index.z && _index.z < div.z);
	}
	// ��ġ�� ���� �ε��� ���
	XMINT3 GetIndex(const XMFLOAT3& _pos);
	// ���� ���
	Sector* GetSector(const XMFLOAT3& _pos);
	Sector* GetSector(const XMINT3& _index);
	// ������Ʈ �߰�
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _pos);
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _index);
	// ������Ʈ ����
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _pos);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _index);
	// ������Ʈ �ٸ� ���ͷ� �̵�
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos);
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, GameObject* _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex);
	// ���� ���������� �����ϴ� �Լ�
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2);
	// �ֺ� ���� ���
	vector<Sector*> GetAroundSectors(const XMINT3& _index);
	// ���������Ұ� �浹�ϴ� ���� ���
	vector<Sector*> GetFrustumSectors(const BoundingFrustum& _frustum);

};
