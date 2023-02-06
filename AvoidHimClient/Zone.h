#pragma once

class GameObject;
class Player;
class PlayScene;
class HitBoxMesh;

class Sector {
	typedef unordered_map<UINT, shared_ptr<GameObject>> Layer;

private:
	vector<Layer> pGameObjectLayers;

public:
	Sector();
	~Sector();

	// ���� ����
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);

	// ��ü ã��
	shared_ptr<GameObject> FindObject(SectorLayer _sectorLayer, UINT _objectID);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _mesh);
	
	// ȸ���� ���� �浹�� Ȯ���ϴ� �Լ�
	vector<shared_ptr<GameObject>> CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor);
	// xz���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionHorizontal(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, shared_ptr<GameObject> _pFloor);
	// y���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionVertical(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, float _timeElapsed = 1.0f);
};

class Zone {
	// GetAroundSectors(const XMINT3& _index)�� �Ҹ��� ���� _index�� ���� �Ҹ���� ������ ���� �� �ִ�.

private:
	shared_ptr<Player> pPlayer;

	vector< shared_ptr<GameObject>> pEnemy;
	// ���� �÷��̾ ���� ������ �ε���
	XMINT3 pindex;
	UINT pid;
	
	XMFLOAT3 startPoint;	// ������ġ
	XMFLOAT3 size;			// �� ������ ũ��
	XMINT3 div;				// ������ ���� ����
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors;
	shared_ptr<PlayScene> pScene;
	unordered_map<string, vector<XMFLOAT4X4>> initVector;

public:
	// ������, �Ҹ���
	Zone();
	Zone(const XMFLOAT3& _size, const XMINT3& _div, shared_ptr<PlayScene> _pScene);
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
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	// ������Ʈ ����
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	// ������Ʈ �ٸ� ���ͷ� �̵�
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos);
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex);
	// ���� ���������� �����ϴ� �Լ�
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2);
	// �ֺ� ���� ���
	vector<Sector*> GetAroundSectors(const XMINT3& _index);
	// ���������Ұ� �浹�ϴ� ���� ���
	vector<Sector*> GetFrustumSectors(const BoundingFrustum& _frustum);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<BoundingFrustum> _pBoundingFrustum);
	void LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	// ȸ���� ���� �浹�� Ȯ���ϴ� �Լ�
	vector<shared_ptr<GameObject>> CheckCollisionRotate(shared_ptr<GameObject> _pFloor = nullptr);

	// xz���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionHorizontal(shared_ptr<GameObject> _pFloor = nullptr);
	// y���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionVertical(float _timeElapsed);
	
	void UpdatePlayerSector();
	void SetPlayer(shared_ptr<Player> _pPlayer) { pPlayer = _pPlayer; };
};
