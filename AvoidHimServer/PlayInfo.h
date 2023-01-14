#pragma once
#include "Timer.h"

template<class _Sector, class _T>
class ZONE {
	// GetAroundSectors(const XMINT3& _index)가 불릴때 같은 _index가 자주 불릴경우 성능을 높일 수 있다.

private:
	XMFLOAT3 startPoint;	// 시작위치
	XMFLOAT3 size;			// 이 공간의 크기
	XMINT3 div;				// 공간을 나눌 개수
	XMFLOAT3 sectorSize;
	vector<vector<vector<_Sector>>> sectors;

public:
	// 생성자, 소멸자
	ZONE() {}
	ZONE( const XMFLOAT3& _size, const XMINT3& _div) : size(_size), div(_div) {
		sectors.assign(div.x, vector<vector<_Sector>>(div.y, vector<_Sector>(div.z, _Sector())));
		sectorSize = Vector3::Division(size, div);
	}
	~ZONE() {

	}

	// get, set 함수
	vector<vector<vector<_Sector>>>& GetAllSectors() {
		return sectors;
	}

	/// 일반함수
	// 인덱스가 범위 내 인지 확인하는 함수
	bool IndexInside(const XMINT3& _index) {
		return (0 <= _index.x && _index.x < div.x) && (0 <= _index.y && _index.y < div.y) && (0 <= _index.z && _index.z < div.z);
	}
	// 위치로 부터 인덱스 얻기
	XMINT3 GetIndex(const XMFLOAT3& _pos) {
		XMINT3 index = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
		index = Vector3::Clamp(index, XMFLOAT3(0, 0, 0), XMFLOAT3(div.x - 1, div.y - 1, div.z - 1));
		return index;
	}
	// 섹터 얻기
	_Sector* GetSector(const XMFLOAT3& _pos) {
		XMINT3 index = GetIndex(_pos);
		return &sectors[index.x][index.y][index.z];
	}
	_Sector* GetSector(const XMINT3& _index) {
		return &sectors[_index.x][_index.y][_index.z];
	}
	// 오브젝트 추가
	void AddObject(_T* _pObject, const XMFLOAT3& _pos, function<void(_T*)> func) {
		_Sector* sector = GetSector(_pos);
		sector->func(_pObject);
	}
	void AddObject(_T* _pObject, const XMINT3& _index, function<void(_T*)> func) {
		_Sector* sector = GetSector(_index);
		sector->func(_pObject);
	}
	// 오브젝트 제거
	void RemoveObject(_T* _pObject, const XMFLOAT3& _pos, function<void(_T*)> func) {
		_Sector* sector = GetSector(_pos);
		sector->func(_pObject);
	}
	void RemoveObject(_T* _pObject, const XMINT3& _index, function<void(_T*)> func) {
		_Sector* sector = GetSector(_index);
		sector->func(_pObject);
	}
	// 오브젝트 다른 섹터로 이동
	void HandOffObject(_T* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos, function<void(_T*)> funcRemove, function<void(_T*)> funcAdd) {
		AddObject(_pObject, _nextPos, funcAdd);
		RemoveObject(_pObject, _prePos, funcRemove);
	}
	void HandOffObject(_T* _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex, function<void(_T*)> funcRemove, function<void(_T*)> funcAdd) {
		AddObject(_pObject, _nextIndex, funcAdd);
		RemoveObject(_pObject, _preIndex, funcRemove);
	}
	// 같은 섹터인지를 리턴하는 함수
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
		return GetIndex(_pos1) == GetIndex(_pos2);
	}
	// 주변 섹터 얻기
	vector<_Sector*> GetAroundSectors(const XMINT3& _index) {
		vector<_Sector*> sectors;
		for (int x = _index.x - 1; x <= _index.x + 1; ++x) {
			for (int y = _index.y - 1; y <= _index.y + 1; ++y) {
				for (int z = _index.z - 1; z <= _index.z + 1; ++z) {
					XMINT3 aroundIndex = XMINT3(x, y, z);
					if (IndexInside(aroundIndex)) {
						sectors.push_back(GetSector(aroundIndex));
					}
				}
			}
		}
		return sectors;
	}
	// 뷰프러스텀과 충돌하는 섹터 얻기
	vector<_Sector*> GetFrustumSectors(const BoundingFrustum& _frustum) {
		// 분할정복을 이용하면 개선가능
		vector<_Sector*> sectors;
		for (int x = 0; x < div.x; ++x) {
			for (int y = 0; y < div.y; ++y) {
				for (int z = 0; z < div.z; ++z) {
					XMUINT3 index = XMUINT3(x, y, z);
					XMFLOAT3 extents = Vector3::ScalarProduct(sectorSize, 0.5f);
					//center = startPoint + index * sectorSize - extend;
					XMFLOAT3 center = Vector3::Subtract(Vector3::Add(startPoint, Vector3::Multiple(sectorSize, index)), extents);
					
					BoundingBox boundingBox(center, extents);
					if (_frustum.Intersects(boundingBox)) {
						sectors.push_back(GetSector(index));
					}
				}
			}
		}
		return sectors;
	}

	// 모든 오브젝트에게 함수수행
	void Play(function<void(float)> func, float _timeElapsed) {
		for (int x = _index.x - 1; x <= _index.x + 1; ++x) {
			for (int y = _index.y - 1; y <= _index.y + 1; ++y) {
				for (int z = _index.z - 1; z <= _index.z + 1; ++z) {
					UINT3 index = UINT3(x, y, z);
					for (_T* pObject : GetSector(index)) {
						pObject->func(_timeElapsed);
					}
				}
			}
		}
	}

};

class PlayInfo {
private:
	UINT playInfoID;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// 플레이어가 로딩을 완료하였는지 확인
	
	ZONE<vector<int>, int> zone;
	Timer timer;
	vector<UINT> participants;
	UINT professorID;

public:
	// 생성자, 소멸자
	PlayInfo(UINT _playInfoID);
	~PlayInfo();

	// Get, Set 함수
	UINT GetID() const { return playInfoID; }

	void SetAllPlayerLoadingComplete(bool _allPlayerLoadingComplete) { allPlayerLoadingComplete = _allPlayerLoadingComplete; }
	bool IsAllPlayerLoadingComplete() const { return allPlayerLoadingComplete; }

	UINT GetNumOfParticipants() const { return (UINT)participants.size(); }
	const vector<UINT>& GetParticipants() const { return participants; }

	UINT GetProfessorID() const { return professorID; }
	void SetProfessorID(UINT _professorID) { professorID = _professorID; }


	// 일반 함수
	void Init(UINT _roomID);
	void LoadingComplete(UINT _clientID);
	void ProcessLoadingComplete();

	//void Escape(UINT _clientID);

};


