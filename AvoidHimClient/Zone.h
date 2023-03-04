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

	// 삽입 삭제
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);

	// 상호작용 물체 포인터 삽입 삭제
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pGameObject);
	// 객체 찾기
	shared_ptr<GameObject> FindObject(SectorLayer _sectorLayer, UINT _objectID);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _mesh);
	
	// 바운딩박스 설정
	void SetBoundingBox(const BoundingBox& _boundingBox);
	const BoundingBox& GetBoundingBox() const;
	// 회전에 대한 충돌을 확인하는 함수
	vector<shared_ptr<GameObject>> CheckCollisionRotate(BoundingOrientedBox& _boundingBox, shared_ptr<GameObject> _pFloor);
	// xz방향 충돌을 확인하는 함수
	shared_ptr<GameObject> CheckCollisionHorizontal(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, shared_ptr<GameObject> _pFloor);
	// y방향 충돌을 확인하는 함수
	shared_ptr<GameObject> CheckCollisionVertical(BoundingOrientedBox& _boundingBox, shared_ptr<Player> _pPlayer, float _timeElapsed = 1.0f);
	// 공격과의 충돌을 처리
	void CheckCollisionWithAttack(shared_ptr<Player> _pPlayer);
	// 투사체와 장애물간의 충돌 처리
	bool CheckCollisionProjectileWithObstacle(const BoundingOrientedBox& _boundingBox);
	// 카메라시야와 벽이 출동하는지 확인
	bool CheckObstacleBetweenPlayerAndCamera(const XMVECTOR& _origin, const XMVECTOR& _direction, float _curDistance);

	pair<float, shared_ptr<InteractObject>> GetNearestInteractObject(const shared_ptr<Player>& _pPlayer);

};

class Zone {
	// GetAroundSectors(const XMINT3& _index)가 불릴때 같은 _index가 자주 불릴경우 성능을 높일 수 있다.
private:
	static UINT objectID;	// 오브젝트의 고유 ID
private:
	shared_ptr<Player> pPlayer;

	vector< shared_ptr<GameObject>> pEnemy;
	// 현재 플레이어가 속한 섹터의 인덱스
	XMINT3 pindex;
	UINT pid;
	
	XMFLOAT3 startPoint;	// 시작위치
	XMFLOAT3 size;			// 이 공간의 크기
	XMINT3 div;				// 공간을 나눌 개수
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors; // x,y,z로 분할된 섹터 공간
	shared_ptr<PlayScene> pScene; // 씬에 대한 포인터
	unordered_map<string, vector<XMFLOAT4X4>> initVector; // 인스턴싱에 필요한 초기 벡터

	unordered_map<UINT, shared_ptr<InteractObject>> pInteractObjTable; // 패킷 도착시 오브젝트를 빠르게 찾기 위한 테이블
	unordered_map<UINT, shared_ptr<Attack>> pAttackObjTable; // 패킷 도착시 오브젝트를 빠르게 찾기 위한 테이블

public:
	// 생성자, 소멸자
	Zone();
	Zone(const XMFLOAT3& _size, const XMINT3& _div, shared_ptr<PlayScene> _pScene);
	~Zone();

	// get, set 함수
	vector<vector<vector<Sector>>>& GetAllSectors() { return sectors; }

	/// 일반함수
	// 인덱스가 범위 내 인지 확인하는 함수
	bool IndexInside(const XMINT3& _index) {
		return (0 <= _index.x && _index.x < div.x) && (0 <= _index.y && _index.y < div.y) && (0 <= _index.z && _index.z < div.z);
	}
	// 위치로 부터 인덱스 얻기
	XMINT3 GetIndex(const XMFLOAT3& _pos);
	// 섹터 얻기
	Sector* GetSector(const XMFLOAT3& _pos);
	Sector* GetSector(const XMINT3& _index);
	// 오브젝트 추가
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void AddInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	
	// 오브젝트 제거
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveInteractObject(UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
	
	// 오브젝트 다른 섹터로 이동
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos);
	void HandOffObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex);
	// 같은 섹터인지를 리턴하는 함수
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2);
	// 주변 섹터 얻기
	vector<Sector*> GetAroundSectors(const XMINT3& _index);

	// 뷰프러스텀과 충돌하는 섹터 얻기
	vector<Sector*> GetFrustumSectors(const BoundingFrustum& _frustum);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<BoundingFrustum> _pBoundingFrustum);
	void LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const array<UINT, MAX_PARTICIPANT>& _enableComputers);
	
	// 회전에 대한 충돌을 확인하는 함수
	vector<shared_ptr<GameObject>> CheckCollisionRotate(shared_ptr<GameObject> _pFloor = nullptr);

	// xz방향 충돌을 확인하는 함수
	shared_ptr<GameObject> CheckCollisionHorizontal(shared_ptr<GameObject> _pFloor = nullptr);
	// y방향 충돌을 확인하는 함수
	shared_ptr<GameObject> CheckCollisionVertical(float _timeElapsed);
	// 플레이어와 공격과의 충돌을 처리
	void CheckCollisionWithAttack();
	// 투사체와 장애물간의 충돌을 처리
	void CheckCollisionProjectileWithObstacle();


	// 카메라가 보는 방향에 물체가 있는지 확인하는 함수
	bool CheckObstacleBetweenPlayerAndCamera(shared_ptr<Camera> _pCamera);
	
	// 현재 플레이어가 상호작용 가능한 오브젝트를 갱신하는 함수
	shared_ptr<InteractObject> UpdateInteractableObject();

	void AddAttack(AttackType _attackType, UINT _objectID, shared_ptr<GameObject> _pPlayerObject, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RemoveAttack(UINT _objectID);
	// 특정 오브젝트에 대한 상호작용을 수행하는 함수
	void Interact(UINT _objectID);
	// 현재 플레이어가 위치한 섹터의 인덱스를 업데이트
	void UpdatePlayerSector();

	void AnimateObjects(float _timeElapsed);
	void SetPlayer(shared_ptr<Player> _pPlayer) { pPlayer = _pPlayer; };
};
