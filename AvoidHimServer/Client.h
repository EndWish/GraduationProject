#pragma once

class Room;
class ClientPlayInfo;

enum class ClientState : unsigned int {
	lobby,				// �κ��ϰ��
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// ��Ⱑ ������ �ٽ� ���ƿ��� ���� ��ٸ��� ����
};

class Client {
private:
	UINT clientID;
	SOCKET socket;
	string name;
	ClientState state;
	Room* pCurrentRoom;
	ClientPlayInfo* pGamePlayInfo;

public:
	// ������ �� �Ҹ���
	Client(SOCKET _socket);
	~Client();

	// Get + Set �Լ�
	SOCKET GetSocket() const { return socket; }
	
	void SetName(const string& _name) { name = _name; }
	const string& GetName() const { return name; }

	void SetClientState(ClientState _state) { state = _state; }
	ClientState GetClientState() const { return state; }

	UINT GetClientID() const { return clientID; }
	void SetClientID(UINT _clientID) { clientID = _clientID; }

	Room* GetCurrentRoom() const { return pCurrentRoom; }
	void SetCurrentRoom(Room* _pRoom) { pCurrentRoom = _pRoom; }

	ClientPlayInfo* GetPlayInfo() const { return pGamePlayInfo; }
	void SetPlayInfo(ClientPlayInfo* _pGamePlayInfo) { pGamePlayInfo = _pGamePlayInfo; }

	// �Ϲ� �Լ�


};

///////////////////////////////////////////////////////////////////////////////
/// ClientPlayInfo
class ClientPlayInfo {
private:
	bool isProfessor;
	float hp, moveSpeed, jumpSpeed;
	int chanceUnlockLab;
	// ���� ������Ʈ ������ ���� ����

public:
	// ������ �� �Ҹ���
	ClientPlayInfo();
	~ClientPlayInfo();

	// Get + Set �Լ�
	bool IsProfessor() const { return isProfessor; }

	// �Ϲ� �Լ�
	void SetProfessor();
	void SetStudent();

};