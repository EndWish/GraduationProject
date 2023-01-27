#pragma once
#include "Client.h"
#include "Room.h"
#include "PlayInfo.h"

class ServerFramework {
private:
	static ServerFramework instance;	// 고유 프레임워크 

public:
	static void Init();
	static void Destroy();
	static ServerFramework& Instance();

private:
	vector<char> buffer;
	
	UINT clientIDCount;
	unordered_map<UINT, Client*> pClients;
	unordered_map<SOCKET, UINT> socketAndIdTable;
	UINT roomIDCount;
	unordered_map<UINT, Room*> pRooms;
	unordered_map<UINT, PlayInfo*> pPlayInfos;

public:
	// 생성자 및 소멸자
	ServerFramework();
	~ServerFramework();

	// Get + Set 함수
	Client* GetClient(int _clientID) { return pClients.contains(_clientID) ? pClients[_clientID] : NULL; }
	Room* GetRoom(int _roomID) { return pRooms.contains(_roomID) ? pRooms[_roomID] : NULL; }

	// 일반 함수
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	void ProcessRecv(SOCKET _socket);
	
	void FrameAdvance();	// 한프레임을 진행하고 필요한경우 메시지를 전송한다.
	
	void AddClient(Client* _pClient);	// 매개변수로 받은 클라이언트를 추가하고 clientID를 부여한다.
	void RemoveClient(UINT _clientID);	// 해당 클라이언트를 메모리해제후 컨테이너에서 제거한다.
	UINT SocketToID(SOCKET _socket);

	void AddRoom(UINT hostID);
	bool RemoveRoom(UINT roomID);
	void CreateRoomlistInfo();	// 방리스트에 대한 정보를 buffer에 저장한다.

	void AddPlayInfo(UINT _roomID);	// PlayInfo를 생성과 초기화를 하고 컨테이너에 추가한다.

	void SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient);

};

