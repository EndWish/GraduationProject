#pragma once

class Room;
class PlayInfo;

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
	bool disconnected;

	string name;
	ClientState state;
	Room* pCurrentRoom;
	PlayInfo* pCurrentPlayInfo;

public:
	// ������ �� �Ҹ���
	Client(SOCKET _socket);
	~Client();

	// Get + Set �Լ�
	SOCKET GetSocket() const { return socket; }

	bool IsDisconnected() const { return disconnected; }
	void SetDisconnected(bool _disconnected) { disconnected = _disconnected; }
	
	const string& GetName() const { return name; }
	void SetName(const string& _name) { name = _name; }

	ClientState GetClientState() const { return state; }
	void SetClientState(ClientState _state) { state = _state; }

	UINT GetClientID() const { return clientID; }
	void SetClientID(UINT _clientID) { clientID = _clientID; }

	Room* GetCurrentRoom() const { return pCurrentRoom; }
	void SetCurrentRoom(Room* _pRoom) { pCurrentRoom = _pRoom; }

	PlayInfo* GetCurrentPlayInfo() const { return pCurrentPlayInfo; }
	void SetCurrentPlayInfo(PlayInfo* _pCurrentPlayInfo) { pCurrentPlayInfo = _pCurrentPlayInfo; }

	// �Ϲ� �Լ�


};