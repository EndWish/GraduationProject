#pragma once
class Client {
private:
	int clientID;
	SOCKET socket;
	string name;

public:
	// ������ �� �Ҹ���
	Client(int _clientID, SOCKET _socket);
	~Client();

	// Get + Set �Լ�
	SOCKET GetSocket() const { return socket; }
	int GetClientID() const { return clientID; }

	// �Ϲ� �Լ�

};

