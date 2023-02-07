#pragma once
#include "Zone.h"

class Client;

class PlayInfo {
private:
	UINT playInfoID;
	UINT objectIDCount;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// �÷��̾ �ε��� �Ϸ��Ͽ����� Ȯ��
	
	//Zone zone;
	vector<pair<UINT, Client*>> participants;	// (=clientID)

	UINT professorObjectID;	// (=objectID)
	unordered_map<UINT, GameObject*> pPlayers;	// objectID-object

	unordered_map<UINT, GameObject*> pDoors;	// objectID-object
	unordered_map<UINT, GameObject*> pLevers;	// objectID-object
	unordered_map<UINT, GameObject*> pWaterDispensers;	// objectID-object

public:
	// ������, �Ҹ���
	PlayInfo(UINT _playInfoID);
	~PlayInfo();

	// Get, Set �Լ�
	UINT GetID() const { return playInfoID; }

	void SetAllPlayerLoadingComplete(bool _allPlayerLoadingComplete) { allPlayerLoadingComplete = _allPlayerLoadingComplete; }
	bool IsAllPlayerLoadingComplete() const { return allPlayerLoadingComplete; }

	UINT GetProfessorObjectID() const { return professorObjectID; }
	void SetProfessorObjectID(UINT _professorObjectID) { professorObjectID = _professorObjectID; }

	const vector<pair<UINT, Client*>>& GetParticipants() const { return participants; }
	const unordered_map<UINT, GameObject*>& GetPlayers() const { return pPlayers; }
	GameObject* GetPlayer(UINT _objectID) { return  pPlayers.contains(_objectID) ? pPlayers[_objectID] : NULL; }

	// �Ϲ� �Լ�
	void Init(UINT _roomID);
	void LoadingComplete(UINT _clientID);
	void ProcessLoadingComplete();

	void FrameAdvance();
	bool ApplyCSPlayerInfo(CS_PLAYER_INFO& _packet);

};


