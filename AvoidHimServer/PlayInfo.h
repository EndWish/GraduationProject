#pragma once
#include "Timer.h"

template<class _Sector, class _T>
class ZONE {
	// GetAroundSectors(const XMINT3& _index)�� �Ҹ��� ���� _index�� ���� �Ҹ���� ������ ���� �� �ִ�.

private:
	XMFLOAT3 startPoint;	// ������ġ
	XMFLOAT3 size;			// �� ������ ũ��
	XMINT3 div;				// ������ ���� ����
	XMFLOAT3 sectorSize;
	vector<vector<vector<_Sector>>> sectors;

public:
	// ������, �Ҹ���
	ZONE() {}
	ZONE( const XMFLOAT3& _size, const XMINT3& _div) : size(_size), div(_div) {
		sectors.assign(div.x, vector<vector<_Sector>>(div.y, vector<_Sector>(div.z, _Sector())));
		sectorSize = Vector3::Division(size, div);
	}
	~ZONE() {

	}

	// get, set �Լ�
	vector<vector<vector<_Sector>>>& GetAllSectors() {
		return sectors;
	}

	/// �Ϲ��Լ�
	// �ε����� ���� �� ���� Ȯ���ϴ� �Լ�
	bool IndexInside(const XMINT3& _index) {
		return (0 <= _index.x && _index.x < div.x) && (0 <= _index.y && _index.y < div.y) && (0 <= _index.z && _index.z < div.z);
	}
	// ��ġ�� ���� �ε��� ���
	XMINT3 GetIndex(const XMFLOAT3& _pos) {
		XMINT3 index = Vector3::Division(Vector3::Subtract(_pos, startPoint), sectorSize);
		index = Vector3::Clamp(index, XMFLOAT3(0, 0, 0), XMFLOAT3(div.x - 1, div.y - 1, div.z - 1));
		return index;
	}
	// ���� ���
	_Sector* GetSector(const XMFLOAT3& _pos) {
		XMINT3 index = GetIndex(_pos);
		return &sectors[index.x][index.y][index.z];
	}
	_Sector* GetSector(const XMINT3& _index) {
		return &sectors[_index.x][_index.y][_index.z];
	}
	// ������Ʈ �߰�
	void AddObject(_T* _pObject, const XMFLOAT3& _pos, function<void(_T*)> func) {
		_Sector* sector = GetSector(_pos);
		sector->func(_pObject);
	}
	void AddObject(_T* _pObject, const XMINT3& _index, function<void(_T*)> func) {
		_Sector* sector = GetSector(_index);
		sector->func(_pObject);
	}
	// ������Ʈ ����
	void RemoveObject(_T* _pObject, const XMFLOAT3& _pos, function<void(_T*)> func) {
		_Sector* sector = GetSector(_pos);
		sector->func(_pObject);
	}
	void RemoveObject(_T* _pObject, const XMINT3& _index, function<void(_T*)> func) {
		_Sector* sector = GetSector(_index);
		sector->func(_pObject);
	}
	// ������Ʈ �ٸ� ���ͷ� �̵�
	void HandOffObject(_T* _pObject, const XMFLOAT3& _prePos, const XMFLOAT3& _nextPos, function<void(_T*)> funcRemove, function<void(_T*)> funcAdd) {
		AddObject(_pObject, _nextPos, funcAdd);
		RemoveObject(_pObject, _prePos, funcRemove);
	}
	void HandOffObject(_T* _pObject, const XMINT3& _preIndex, const XMINT3& _nextIndex, function<void(_T*)> funcRemove, function<void(_T*)> funcAdd) {
		AddObject(_pObject, _nextIndex, funcAdd);
		RemoveObject(_pObject, _preIndex, funcRemove);
	}
	// ���� ���������� �����ϴ� �Լ�
	bool IsSameSector(const XMFLOAT3& _pos1, const XMFLOAT3& _pos2) {
		return GetIndex(_pos1) == GetIndex(_pos2);
	}
	// �ֺ� ���� ���
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
	// ���������Ұ� �浹�ϴ� ���� ���
	vector<_Sector*> GetFrustumSectors(const BoundingFrustum& _frustum) {
		// ���������� �̿��ϸ� ��������
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

	// ��� ������Ʈ���� �Լ�����
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
	unordered_map<UINT, bool> loadingCompletes;	// �÷��̾ �ε��� �Ϸ��Ͽ����� Ȯ��
	
	ZONE<vector<int>, int> zone;
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


