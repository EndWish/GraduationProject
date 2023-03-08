#pragma once
#include "Client.h"
#include "Room.h"
#include "PlayInfo.h"

class ServerFramework {
private:
	static ServerFramework instance;	// ���� �����ӿ�ũ 

public:
	static ServerFramework& Instance();

private:
	HWND windowHandle;

	chrono::system_clock::time_point lastTime;

	UINT clientIDCount;
	unordered_map<UINT, Client*> pClients;
	unordered_map<SOCKET, UINT> socketAndIdTable;
	UINT roomIDCount;
	unordered_map<UINT, Room*> pRooms;
	unordered_map<UINT, PlayInfo*> pPlayInfos;

	// �������� �̸� �о�´�.
	vector<pair<UINT, GameObject*>> pInitialObjects;
	vector<XMFLOAT3> studentStartPositions;
	XMFLOAT3 professorStartPosition;
	
public:
	// ������ �� �Ҹ���
	ServerFramework();
	~ServerFramework();

	void Init(HWND _windowHandle);
	void Destroy();

	// Get + Set �Լ�
	Client* GetClient(int _clientID) { return pClients.contains(_clientID) ? pClients[_clientID] : NULL; }
	Room* GetRoom(int _roomID) { return pRooms.contains(_roomID) ? pRooms[_roomID] : NULL; }

	// �Ϲ� �Լ�
	void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	void ProcessRecv(SOCKET _socket);
	
	void FrameAdvance();	// ���������� �����ϰ� �ʿ��Ѱ�� �޽����� �����Ѵ�.
	
	void AddClient(Client* _pClient);	// �Ű������� ���� Ŭ���̾�Ʈ�� �߰��ϰ� clientID�� �ο��Ѵ�.
	void RemoveClient(UINT _clientID);	// �ش� Ŭ���̾�Ʈ�� �޸������� �����̳ʿ��� �����Ѵ�.
	UINT SocketToID(SOCKET _socket);

	void AddRoom(UINT hostID);
	bool RemoveRoom(UINT roomID);
	void SendRoomlistInfo(Client* _pClient,int _page);	// �渮��Ʈ�� ���� ������ buffer�� �����Ѵ�.

	void AddPlayInfo(UINT _roomID);	// PlayInfo�� ������ �ʱ�ȭ�� �ϰ� �����̳ʿ� �߰��Ѵ�.

	void SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient);

	void LoadMapFile();
	const vector<pair<UINT, GameObject*>>& GetinitialObjects() const { return pInitialObjects; };
	const vector<XMFLOAT3>& GetShuffledStudentStartPositions();
	XMFLOAT3 GetProfessorStartPosition() { return professorStartPosition; }

};

