#pragma once

enum class ClientState : unsigned int {
	lobby,				// 로비일경우
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// 경기가 끝나고 다시 돌아오는 것을 기다리는 상태
};

class Client {
private:
	int clientID;
	SOCKET socket;
	string name;
	ClientState state;
	int currentRoom;	// 룸이 아닌경우 -1

public:
	// 생성자 및 소멸자
	Client(SOCKET _socket);
	~Client();

	// Get + Set 함수
	SOCKET GetSocket() const { return socket; }
	
	void SetName(const string& _name) { name = _name; }
	const string& GetName() const { return name; }

	void SetClientState(ClientState _state) { state = _state; }
	ClientState GetClientState() const { return state; }

	int GetClientID() const { return clientID; }
	void SetClientID(int _clientID) { clientID = _clientID; }

	// 일반 함수


};

