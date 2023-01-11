#pragma once

class Room;
class ClientPlayInfo;

enum class ClientState : unsigned int {
	lobby,				// 로비일경우
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// 경기가 끝나고 다시 돌아오는 것을 기다리는 상태
};

class Client {
private:
	UINT clientID;
	SOCKET socket;
	string name;
	ClientState state;
	Room* pCurrentRoom;
	ClientPlayInfo* pGamePlayInfo;

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

	UINT GetClientID() const { return clientID; }
	void SetClientID(UINT _clientID) { clientID = _clientID; }

	Room* GetCurrentRoom() const { return pCurrentRoom; }
	void SetCurrentRoom(Room* _pRoom) { pCurrentRoom = _pRoom; }

	ClientPlayInfo* GetPlayInfo() const { return pGamePlayInfo; }
	void SetPlayInfo(ClientPlayInfo* _pGamePlayInfo) { pGamePlayInfo = _pGamePlayInfo; }

	// 일반 함수


};

///////////////////////////////////////////////////////////////////////////////
/// ClientPlayInfo
class ClientPlayInfo {
private:
	bool isProfessor;
	float hp, moveSpeed, jumpSpeed;
	int chanceUnlockLab;
	// 게임 오브젝트 계층에 대한 정보

public:
	// 생성자 및 소멸자
	ClientPlayInfo();
	~ClientPlayInfo();

	// Get + Set 함수
	bool IsProfessor() const { return isProfessor; }

	// 일반 함수
	void SetProfessor();
	void SetStudent();

};