#pragma once

class GameObject;
class PlayScene;

class Sector {
	typedef unordered_map<UINT, shared_ptr<GameObject>> Layer;

private:
	vector<Layer> pGameObjectLayers;

public:
	Sector();
	~Sector();

	// 삽입 삭제
	void AddObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pGameObject);

	// 객체 찾기
	shared_ptr<GameObject> FindObject(SectorLayer _sectorLayer, UINT _objectID);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class Zone {
	// GetAroundSectors(const XMINT3& _index)가 불릴때 같은 _index가 자주 불릴경우 성능을 높일 수 있다.

private:
	XMFLOAT3 startPoint;	// 시작위치
	XMFLOAT3 size;			// 이 공간의 크기
	XMINT3 div;				// 공간을 나눌 개수
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors;
	shared_ptr<PlayScene> pScene;
	unordered_map<string, vector<XMFLOAT4X4>> initVector;
public:
	// 생성자, 소멸자
	Zone();
	Zone(const XMFLOAT3& _size, const XMINT3& _div);
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
	// 오브젝트 제거
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMFLOAT3& _pos);
	void RemoveObject(SectorLayer _sectorLayer, UINT _objectID, shared_ptr<GameObject> _pObject, const XMINT3& _index);
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
	void LoadZoneFromFile(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};
