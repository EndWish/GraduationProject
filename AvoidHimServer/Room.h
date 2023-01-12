#pragma once

class Room {
private:
	//static const int maxParticipant = 5;	// 최대 참가자 수
private:
	UINT id;
	UINT hostID;
	vector<UINT> participants;
	bool gameRunning;

public:
	// 생성자 및 소멸자
	Room(UINT _id);
	~Room();

	// Get + Set 함수
	UINT GetID() const { return id; }
	UINT GetNumOfParticipants() const { return (UINT)participants.size(); }
	const vector<UINT>& GetParticipants() const { return participants; }
	UINT GetHostID() const { return hostID; }
	bool IsGameRunning() const { return gameRunning; }
	void SetGameRunning(bool _gameRuning) { gameRunning = _gameRuning; }

	// 일반함수
	bool EnterUser(UINT _clientID);	// 인원이 가득찼다면 false를 리턴한다.
	void LeaveUser(UINT _clientID);	// 해당 함수를 호출하면 방의 인원을 확인하여 방을 삭제할지 판단한다.
	
	void GameStart();

};