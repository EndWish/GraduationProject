#pragma once

class Room {
private:
	//static const int maxParticipant = 5;	// �ִ� ������ ��
private:
	UINT id;
	UINT hostID;
	vector<UINT> participants;
	bool gameRunning;

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

	// �Ϲ��Լ�
	bool EnterUser(UINT _clientID);	// �ο��� ����á�ٸ� false�� �����Ѵ�.
	void LeaveUser(UINT _clientID);	// �ش� �Լ��� ȣ���ϸ� ���� �ο��� Ȯ���Ͽ� ���� �������� �Ǵ��Ѵ�.
	
	void GameStart();

};