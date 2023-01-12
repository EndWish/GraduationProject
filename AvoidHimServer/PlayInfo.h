#pragma once
#include "Timer.h"

class PlayInfo {
private:
	UINT playInfoID;
	bool allPlayerLoadingComplete;
	unordered_map<UINT, bool> loadingCompletes;	// �÷��̾ �ε��� �Ϸ��Ͽ����� Ȯ��

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

