#pragma once
#include "Client.h"
#include "Room.h"

class ServerFramework {
private:
	static const int bufferSize = 1024;
	static ServerFramework instance;	// 고유 프레임워크 

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
	// 생성자 및 소멸자
	ServerFramework();
	~ServerFramework();

	// Get + Set 함수
	Client& GetClient(int _clientID) { return *pClients[_clientID]; }

	// 일반 함수
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	void ProcessRecv(SOCKET _socket);
	
	void FrameAdvance();	// 한프레임을 진행하고 필요한경우 메시지를 전송한다.
	void AddClient(Client* _pClient);	// 매개변수로 받은 클라이언트를 추가하고 clientID를 부여한다.
	void RemoveClient(int _clientID);	// 해당 클라이언트를 메모리해제후 컨테이너에서 제거한다.
};

