#pragma once
#include "Client.h"

class ServerFramework {
private:
	static ServerFramework instance;	// 고유 프레임워크 

public:
	static void Init();
	static void Destroy();
	static ServerFramework& Instance();

private:
	int clientIDCount;
	unordered_map<SOCKET, Client*> pClients;

public:
	// 생성자 및 소멸자
	ServerFramework();
	~ServerFramework();

	// Get + Set 함수


	// 일반 함수
	void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void FrameAdvance();
	void AddClient(Client* _pClient);
	void RemoveClient(const SOCKET& _socket);

};

