#pragma once
#include "Client.h"

class ServerFramework {
private:
	static ServerFramework instance;	// ���� �����ӿ�ũ 

public:
	static void Init();
	static void Destroy();
	static ServerFramework& Instance();

private:
	int clientIDCount;
	unordered_map<SOCKET, Client*> pClients;

public:
	// ������ �� �Ҹ���
	ServerFramework();
	~ServerFramework();

	// Get + Set �Լ�


	// �Ϲ� �Լ�
	void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void FrameAdvance();
	void AddClient(Client* _pClient);
	void RemoveClient(const SOCKET& _socket);

};

