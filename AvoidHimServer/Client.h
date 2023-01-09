#pragma once

enum class ClientState : unsigned int {
	lobby,				// �κ��ϰ��
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// ��Ⱑ ������ �ٽ� ���ƿ��� ���� ��ٸ��� ����
};

class Client {
private:
	int clientID;
	SOCKET socket;
	string name;
	ClientState state;
	int currentRoom;	// ���� �ƴѰ�� -1

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

	int GetClientID() const { return clientID; }
	void SetClientID(int _clientID) { clientID = _clientID; }

	// �Ϲ� �Լ�


};

