#pragma once
#include "Zone.h"
#include "GameObject.h"

class Client;

#pragma pack(push, 1)
struct READ_CID_IN_PACKET {
	CS_PACKET_TYPE packetType = CS_PACKET_TYPE::none;
	UINT cid = 0;
};
#pragma pack(pop)

class PlayInfo {
private:
	UINT playInfoID;
	UINT objectIDCount;
	bool allPlayerLoadingComplete;
	UINT prisonDoorObjectID;
	unordered_map<UINT, bool> loadingCompletes;	// 플레이어가 로딩을 완료하였는지 확인
	
	//Zone zone;
	vector<pair<UINT, Client*>> participants;	// (=clientID)

	UINT professorClientID;	// (=clientID)
	UINT professorObjectID;	// (=objectID)
	unordered_map<UINT, Player*> pPlayers;	// objectID-object

	unordered_map<UINT, Door*> pDoors;	// objectID-object
	unordered_map<UINT, Lever*> pLevers;	// objectID-object
	unordered_map<UINT, WaterDispenser*> pWaterDispensers;	// objectID-object
	unordered_map<UINT, Computer*> pComputers;	// objectID-object

	UINT itemCount; // 해당 게임에 현재 존재하는 아이템의 수
	vector<bool> isExistItem;	// 해당 인덱스 스폰 포지션에 아이템이 있는지 판단
	bool hackingComplete;
	
	float itemCreateCoolTime;
	bool allLeverPowerOn;

	chrono::system_clock::time_point gameStartTime;
	bool endGame;

public:
	// 생성자, 소멸자
	PlayInfo(UINT _playInfoID);
	~PlayInfo();

	// Get, Set 함수
	UINT GetID() const { return playInfoID; }

	void SetAllPlayerLoadingComplete(bool _allPlayerLoadingComplete) { allPlayerLoadingComplete = _allPlayerLoadingComplete; }
	bool IsAllPlayerLoadingComplete() const { return allPlayerLoadingComplete; }

	UINT GetProfessorObjectID() const { return professorObjectID; }
	void SetProfessorObjectID(UINT _professorObjectID) { professorObjectID = _professorObjectID; }

	const vector<pair<UINT, Client*>>& GetParticipants() const { return participants; }
	const unordered_map<UINT, Player*>& GetPlayers() const { return pPlayers; }
	GameObject* GetPlayer(UINT _objectID) { return  pPlayers.contains(_objectID) ? pPlayers[_objectID] : NULL; }

	// 일반 함수
	void Init(UINT _roomID);
	void LoadingComplete(UINT _clientID);
	void ProcessLoadingComplete();

	void FrameAdvance(float _timeElapsed);
	void ProcessRecv(CS_PACKET_TYPE _packetType);

	void AddItem();

	void EscapeClient(UINT _clientID);
};


