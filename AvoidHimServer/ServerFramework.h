#pragma once
#include "Client.h"
#include "Room.h"

class ServerFramework {
private:
	static const int bufferSize = 1024;
	static ServerFramework instance;	// ���� �����ӿ�ũ 

public:
	static void Init();
	static void Destroy();
	static ServerFramework& Instance();

private:
	char buffer[bufferSize];

	int clientIDCount;
	unordered_map<int, Client*> pClients;
	int roomIDCount;
	vector<Room*> pRooms;

public:
	// ������ �� �Ҹ���
	ServerFramework();
	~ServerFramework();

	// Get + Set �Լ�
	Client& GetClient(int _clientID) { return *pClients[_clientID]; }

	// �Ϲ� �Լ�
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	void ProcessRecv(SOCKET _socket);
	
	void FrameAdvance();	// ���������� �����ϰ� �ʿ��Ѱ�� �޽����� �����Ѵ�.
	void AddClient(Client* _pClient);	// �Ű������� ���� Ŭ���̾�Ʈ�� �߰��ϰ� clientID�� �ο��Ѵ�.
	void RemoveClient(int _clientID);	// �ش� Ŭ���̾�Ʈ�� �޸������� �����̳ʿ��� �����Ѵ�.
};

