#pragma once
#include "Zone.h"

class Client;

class PlayInfo {
private:
	UINT playInfoID;
	UINT objectIDCount;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// 플레이어가 로딩을 완료하였는지 확인
	
	//Zone zone;
	vector<pair<UINT, Client*>> participants;	// (=clientID)

	UINT professorObjectID;	// (=objectID)
	unordered_map<UINT, GameObject*> pPlayers;	// objectID-object

	unordered_map<UINT, GameObject*> pDoors;	// objectID-object
	unordered_map<UINT, GameObject*> pLevers;	// objectID-object
	unordered_map<UINT, GameObject*> pWaterDispensers;	// objectID-object

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
	const unordered_map<UINT, GameObject*>& GetPlayers() const { return pPlayers; }
	GameObject* GetPlayer(UINT _objectID) { return  pPlayers.contains(_objectID) ? pPlayers[_objectID] : NULL; }

	// 일반 함수
	void Init(UINT _roomID);
	void LoadingComplete(UINT _clientID);
	void ProcessLoadingComplete();

	void FrameAdvance();
	bool ApplyCSPlayerInfo(CS_PLAYER_INFO& _packet);

};


