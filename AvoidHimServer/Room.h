#pragma once
#include "Timer.h"

class RoomPlayInfo;

class Room {
private:
	//static const int maxParticipant = 5;	// �ִ� ������ ��
private:
	UINT id;
	UINT hostID;
	vector<UINT> participants;

	bool gameRunning;
	RoomPlayInfo* playInfo;

public:
	// ������ �� �Ҹ���
	Room(UINT _id);
	~Room();

	// Get + Set �Լ�
	UINT GetID() const { return id; }
	UINT GetNumOfParticipants() const { return (UINT)participants.size(); }
	const vector<UINT>& GetParticipants() const { return participants; }
	UINT GetHostID() const { return hostID; }
	bool IsGameRunning() const { return gameRunning; }
	void SetGameRunning(bool _gameRuning) { gameRunning = _gameRuning; }
	RoomPlayInfo* GetPlayInfo() const { return playInfo; }
	void SetPlayInfo(RoomPlayInfo* _playInfo) { playInfo = _playInfo; }

	// �Ϲ��Լ�
	bool EnterUser(UINT _clientID);	// �ο��� ����á�ٸ� false�� �����Ѵ�.
	void LeaveUser(UINT _clientID);	// �ش� �Լ��� ȣ���ϸ� ���� �ο��� Ȯ���Ͽ� ���� �������� �Ǵ��Ѵ�.
	
	void GameStart();

};

class RoomPlayInfo {

private:
	UINT professorID;
	Timer timer;

public:
	UINT GetProfessorID() { return professorID; }
	void SetProfessorID(UINT _professorID) { professorID = _professorID; }

};