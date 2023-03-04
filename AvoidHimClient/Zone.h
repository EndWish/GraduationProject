#pragma once
#include "GameObject.h"
#include "Camera.h"

class Player;
class PlayScene;
class HitBoxMesh;

class Sector {
	typedef unordered_map<UINT, shared_ptr<GameObject>> Layer;

private:
	BoundingBox boundingBox;
	vector<Layer> pGameObjectLayers;
	unordered_map<UINT, shared_ptr<InteractObject>> pInteractionObjects;
public:
	Sector();
	~Sector();

	// ���� ����
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);

	// ��ȣ�ۿ� ��ü ������ ���� ����
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject);
	// ��ü ã��
	shared_ptr<GameObject> FindObject(SectorLayer _sectorLayer, UINT _objectID);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _mesh);
	
	// �ٿ���ڽ� ����
	void SetBoundingBox(const BoundingBox& _boundingBox);
	const BoundingBox& GetBoundingBox() const;
	// ȸ���� ���� �浹�� Ȯ���ϴ� �Լ�
	vector<shared_ptr<GameObject>> CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor);
	// xz���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionHorizontal(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, shared_ptr<GameObject> _pFloor);
	// y���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionVertical(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, float _timeElapsed = 1.0f);
	// ���ݰ��� �浹�� ó��
	void CheckCollisionWithAttack(shared_ptr<Player> _pPlayer);
	// ����ü�� ��ֹ����� �浹 ó��
	bool CheckCollisionProjectileWithObstacle(const BoundingOrientedBox& _boundingBox);
	// ī�޶�þ߿� ���� �⵿�ϴ��� Ȯ��
	bool CheckObstacleBetweenPlayerAndCamera(const XMVECTOR& _origin, const XMVECTOR& _direction, float _curDistance);

	pair<float, shared_ptr<InteractObject>> GetNearestInteractObject(const shared_ptr<Player>& _pPlayer);

};

class Zone {
	// GetAroundSectors(const XMINT3& _index)�� �Ҹ��� ���� _index�� ���� �Ҹ���� ������ ���� �� �ִ�.
private:
	static UINT objectID;	// ������Ʈ�� ���� ID
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
	vector<vector<vector<Sector>>> sectors; // x,y,z�� ���ҵ� ���� ����
	shared_ptr<PlayScene> pScene; // ���� ���� ������
	unordered_map<string, vector<XMFLOAT4X4>> initVector; // �ν��Ͻ̿� �ʿ��� �ʱ� ����

	unordered_map<UINT, shared_ptr<InteractObject>> pInteractObjTable; // ��Ŷ ������ ������Ʈ�� ������ ã�� ���� ���̺�
	unordered_map<UINT, shared_ptr<Attack>> pAttackObjTable; // ��Ŷ ������ ������Ʈ�� ������ ã�� ���� ���̺�

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
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	
	// ������Ʈ ����
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	
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
	void LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const array<UINT, MAX_PARTICIPANT>& _enableComputers);
	
	// ȸ���� ���� �浹�� Ȯ���ϴ� �Լ�
	vector<shared_ptr<GameObject>> CheckCollisionRotate(shared_ptr<GameObject> _pFloor = nullptr);

	// xz���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionHorizontal(shared_ptr<GameObject> _pFloor = nullptr);
	// y���� �浹�� Ȯ���ϴ� �Լ�
	shared_ptr<GameObject> CheckCollisionVertical(float _timeElapsed);
	// �÷��̾�� ���ݰ��� �浹�� ó��
	void CheckCollisionWithAttack();
	// ����ü�� ��ֹ����� �浹�� ó��
	void CheckCollisionProjectileWithObstacle();


	// ī�޶� ���� ���⿡ ��ü�� �ִ��� Ȯ���ϴ� �Լ�
	bool CheckObstacleBetweenPlayerAndCamera(shared_ptr<Camera> _pCamera);
	
	// ���� �÷��̾ ��ȣ�ۿ� ������ ������Ʈ�� �����ϴ� �Լ�
	shared_ptr<InteractObject> UpdateInteractableObject();

	void AddAttack(AttackType _attackType, UINT _objectID, shared_ptr<GameObject> _pPlayerObject, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RemoveAttack(UINT _objectID);
	// Ư�� ������Ʈ�� ���� ��ȣ�ۿ��� �����ϴ� �Լ�
	void Interact(UINT _objectID);
	// ���� �÷��̾ ��ġ�� ������ �ε����� ������Ʈ
	void UpdatePlayerSector();

	void AnimateObjects(float _timeElapsed);
	void SetPlayer(shared_ptr<Player> _pPlayer) { pPlayer = _pPlayer; };
};
