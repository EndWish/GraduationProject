#pragma once
class Client {
private:
	int clientID;
	SOCKET socket;
	string name;

public:
	// 생성자 및 소멸자
	Client(int _clientID, SOCKET _socket);
	~Client();

	// Get + Set 함수
	SOCKET GetSocket() const { return socket; }
	int GetClientID() const { return clientID; }

	// 일반 함수

};

