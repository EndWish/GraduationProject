#pragma once
#include "Timer.h"

template<class T>
class ZONE {
	typedef vector<T*> Sector;

private:
	XMFLOAT3 startPoint;	// ������ġ
	XMFLOAT3 size;	// �� ������ ũ��
	XMUINT3 div;		// ������ ���� ����
	XMFLOAT3 sectorSize;
	vector<vector<vector<Sector>>> sectors;

public:
	// ������, �Ҹ���
	ZONE() {}
	ZONE( const XMFLOAT3& _size, const XMUINT3& _div) : size(_size), div(_div) {
		sectors.assign(div.x, vector<vector<Sector>>(div.y, vector<Sector>(div.z, Sector())));
		sectorSize = Vector3::Division(size, div);
	}
	~ZONE() {

	}

	// get, set �Լ�


	/// �Ϲ��Լ�
	// ��ġ�� ���� �ε��� ���
	XMUINT3 GetIndex(const XMFLOAT3& _pos) {
		XMUINT3 index = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
		index = Vector3::Clamp(index, XMFLOAT3(0, 0, 0), XMFLOAT3(div.x - 1, div.y - 1, div.z - 1));
		return index;
	}
	// ���� ���
	Sector* GetSector(const XMFLOAT3& _pos) {
		XMUINT3 index = GetIndex(_pos);
		return sectors[index.x][index.y][index.z];
	}
	Sector* GetSector(const XMUINT3& _index) {
		return sectors[_index.x][_index.y][_index.z];
	}
	// ������Ʈ �߰�
	void AddObject(T* _pObject, const XMFLOAT3& _pos) {
		Sector& sector = GetSector(_pos);
		sector.push_back(_pObject);
	}
	void AddObject(T* _pObject, const XMUINT3& _index) {
		Sector& sector = GetSector(_index);
		sector.push_back(_pObject);
	}
	// ������Ʈ ����
	void RemoveObject(T* _pObject, const XMFLOAT3& _pos) {
		Sector& sector = GetSector(_pos);
		sector.erase(ranges::find(sector, _pObject));
	}
	void RemoveObject(T* _pObject, const XMUINT3& _index) {
		Sector& sector = GetSector(_index);
		sector.erase(ranges::find(sector, _pObject));
	}
	// ������Ʈ �ٸ� ���ͷ� �̵�
	void HandOffObject(T* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos) {
		AddObject(_pObject, _nextPos);
		RemoveObject(_pObject, _prePos);
	}
	void HandOffObject(T* _pObject, const XMUINT3& _preIndex, const XMUINT3& _nextIndex) {
		AddObject(_pObject, _nextIndex);
		RemoveObject(_pObject, _preIndex);
	}
	// ���� ���������� �����ϴ� �Լ�
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
		return GetIndex(_pos1) == GetIndex(_pos2);
	}
	// �ֺ� ���� ����
	array<Sector*, 8> Get8xSectors(const XMFLOAT3& _pos) {
		// [������]
	}

};

class PlayInfo {
private:
	UINT playInfoID;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// �÷��̾ �ε��� �Ϸ��Ͽ����� Ȯ��
	
	ZONE<int> zone;
	Timer timer;
	vector<UINT> participants;
	UINT professorID;

public:
	// ������, �Ҹ���
	PlayInfo(UINT _playInfoID);
	~PlayInfo();

	// Get, Set �Լ�
	UINT GetID() const { return playInfoID; }

	void SetAllPlayerLoadingComplete(bool _allPlayerLoadingComplete) { allPlayerLoadingComplete = _allPlayerLoadingComplete; }
	bool IsAllPlayerLoadingComplete() const { return allPlayerLoadingComplete; }

	UINT GetNumOfParticipants() const { return (UINT)participants.size(); }
	const vector<UINT>& GetParticipants() const { return participants; }

	UINT GetProfessorID() const { return professorID; }
	void SetProfessorID(UINT _professorID) { professorID = _professorID; }


	// �Ϲ� �Լ�
	void Init(UINT _roomID);
	void LoadingComplete(UINT _clientID);
	void ProcessLoadingComplete();

	//void Escape(UINT _clientID);

};


