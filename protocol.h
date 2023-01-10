#pragma once
// ���� ���� ��� ����

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define WM_SOCKET  (WM_USER+1)

const int maxParticipant = 5;

///////////////////////////////

enum class CS_PACKET_TYPE : char {
	makeRoom, queryRoomlistInfo, visitRoom, outRoom
};

enum class SC_PACKET_TYPE : char {
	giveClientID, roomInfo, roomPlayersInfo, roomVisitPlayerInfo, roomOutPlayerInfo, roomVisitFail
};

#pragma pack(push, 1)

// Ŭ�� -> ����
struct CS_MAKE_ROOM {	// ���� ����� �޶�� ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::makeRoom;
	UINT hostID = 0;
};
struct CS_QUERY_ROOMLIST_INFO {	// �κ񿡼� �濡���� ������ �䱸�Ҷ� ������ ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::queryRoomlistInfo;
	UINT cid = 0;
};
struct CS_QUERY_VISIT_ROOM {	// �濡 �����Ҷ� �����ϴ� ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::visitRoom;
	UINT cid = 0;
	UINT visitRoomID = 0;
};
struct CS_QUERY_OUT_ROOM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::outRoom;
	UINT cid = 0;
};

/// ����->Ŭ��
struct SC_GIVE_CLIENT_ID {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::giveClientID;
	UINT clientID = 0;
};
struct SC_SUB_ROOMLIST_INFO {
	UINT roomID = 0;
	UINT nParticipant = 0;
	// array<char, 50> roomName;
};
struct SC_ROOMLIST_INFO {	// �κ񿡼� ���� �ʿ��� ��鿡 ���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomInfo;
	UINT nRoom = 0;
	// nRoom ������ŭ "SC_SUB_ROOMLIST_INFO"�� �����Ѵ�.
};

struct SC_SUB_ROOM_PLAYERS_INFO {
	UINT cid = 0;
	bool ready = false;
	// array<char, 20> name;
};
struct SC_ROOM_PLAYERS_INFO {	// �濡 ���������� �����ϴ� �÷��̾���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomPlayersInfo;
	UINT hostID = 0;
	UINT nParticipant = 0;
	SC_SUB_ROOM_PLAYERS_INFO participantInfos[maxParticipant];
};

struct SC_ROOM_VISIT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomVisitPlayerInfo;
	UINT cid = 0;
	// array<char, 20> name;
};

struct SC_ROOM_OUT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomOutPlayerInfo;
	UINT cid = 0;
};

struct SC_ROOM_VISIT_FAIL {		// �� ���忡 �������� ��� �����ִ� ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomVisitFail;
	int cause = -1;	// ���� (0 : ���� �������� ����, 1 : �ο��ʰ�)
};


#pragma pack(pop)