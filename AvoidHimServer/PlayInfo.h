#pragma once
#include "Timer.h"

template<class T>
class ZONE {
	typedef vector<T*> Sector;

private:
	XMFLOAT3 startPoint;	// 시작위치
	XMFLOAT3 size;	// 이 공간의 크기
	XMUINT3 div;		// 공간을 나눌 개수
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors;

public:
	// 생성자, 소멸자
	ZONE() {}
	ZONE( const XMFLOAT3& _size, const XMUINT3& _div) : size(_size), div(_div) {
		sectors.assign(div.x, vector<vector<Sector>>(div.y, vector<Sector>(div.z, Sector())));
		sectorSize = Vector3::Division(size, div);
	}
	~ZONE() {

	}

	// get, set 함수


	/// 일반함수
	// 위치로 부터 인덱스 얻기
	XMUINT3 GetIndex(const XMFLOAT3& _pos) {
		XMUINT3 index = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
		index = Vector3::Clamp(index, XMFLOAT3(0, 0, 0), XMFLOAT3(div.x - 1, div.y - 1, div.z - 1));
		return index;
	}
	// 섹터 얻기
	Sector* GetSector(const XMFLOAT3& _pos) {
		XMUINT3 index = GetIndex(_pos);
		return sectors[index.x][index.y][index.z];
	}
	Sector* GetSector(const XMUINT3& _index) {
		return sectors[_index.x][_index.y][_index.z];
	}
	// 오브젝트 추가
	void AddObject(T* _pObject, const XMFLOAT3& _pos) {
		Sector& sector = GetSector(_pos);
		sector.push_back(_pObject);
	}
	void AddObject(T* _pObject, const XMUINT3& _index) {
		Sector& sector = GetSector(_index);
		sector.push_back(_pObject);
	}
	// 오브젝트 제거
	void RemoveObject(T* _pObject, const XMFLOAT3& _pos) {
		Sector& sector = GetSector(_pos);
		sector.erase(ranges::find(sector, _pObject));
	}
	void RemoveObject(T* _pObject, const XMUINT3& _index) {
		Sector& sector = GetSector(_index);
		sector.erase(ranges::find(sector, _pObject));
	}
	// 오브젝트 다른 섹터로 이동
	void HandOffObject(T* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
		AddObject(_pObject, _nextPos);
		RemoveObject(_pObject, _prePos);
	}
	void HandOffObject(T* _pObject, const XMUINT3& _preIndex, const XMUINT3& _nextIndex) {
		AddObject(_pObject, _nextIndex);
		RemoveObject(_pObject, _preIndex);
	}
	// 같은 섹터인지를 리턴하는 함수
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
		return GetIndex(_pos1) == GetIndex(_pos2);
	}
	// 주변 섹터 리턴
	array<Sector*, 8> Get8xSectors(const XMFLOAT3& _pos) {
		// [진행중]
	}

};

class PlayInfo {
private:
	UINT playInfoID;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// 플레이어가 로딩을 완료하였는지 확인
	
	ZONE<int> zone;
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


