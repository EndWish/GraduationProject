#pragma once
// ���� ���� ��� ����

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define WM_SOCKET  (WM_USER+1)

#include <array>
#include <DirectXMath.h>
using namespace DirectX;

const int maxParticipant = 5;

///////////////////////////////

enum class CS_PACKET_TYPE : char {
	makeRoom = 1, queryRoomlistInfo, visitRoom, outRoom, ready, loadingComplete, playerInfo, aniClipChange
};

enum class SC_PACKET_TYPE : char {
	giveClientID = 1, roomListInfo, roomPlayersInfo, roomVisitPlayerInfo, roomOutPlayerInfo, fail, 
	ready, gameStart, allPlayerLoadingComplete, playerInfo, aniClipChange

};

enum class SC_FAIL_TYPE : int {
	unknown = 1,	//  �ʱⰪ
	noExistRoom,	// ���� �������� ������
	roomOvercapacity,	// �濡 ������ �ʰ�������
	lackOfParticipants,	// �濡 �����ڰ� �����Ҷ�
	notAllReady,	// �غ����� ���� �ο��� �����Ҷ�
	roomGameStarted,	// �̹� ������ ���۵Ǿ��� ���
};

enum class SectorLayer : char {
	player,
	obstacle,
	attack,
	num
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
struct CS_OUT_ROOM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::outRoom;
	UINT cid = 0;
};
struct CS_READY {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::ready;
	UINT cid = 0;
};
struct CS_LOADING_COMPLETE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::loadingComplete;
	UINT cid = 0;
	UINT roomID = 0;
};
struct CS_PLALYER_INFO {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::playerInfo;
	UINT cid = 0;
	XMFLOAT3 position = XMFLOAT3(0,0,0);
	XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT3 size = XMFLOAT3(1, 1, 1);
	float aniTime = 0.0f;
};
struct CS_ANICLIP_CHANGE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::aniClipChange;
	UINT cid = 0;
	std::array<char, 30> clipName;
};

/// ����->Ŭ��
struct SC_GIVE_CLIENT_ID {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::giveClientID;
	UINT clientID = 0;
};
struct SC_SUB_ROOMLIST_INFO {
	UINT roomID = 0;
	UINT nParticipant = 0;
	bool started = false;
	// std::array<char, 50> roomName;
};
struct SC_ROOMLIST_INFO {	// �κ񿡼� ���� �ʿ��� ��鿡 ���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomListInfo;
	UINT nRoom = 0;
	// nRoom ������ŭ "SC_SUB_ROOMLIST_INFO"�� �����Ѵ�.
};
struct SC_SUB_ROOM_PLAYERS_INFO {
	UINT clientID = 0;
	bool ready = false;
	// std::array<char, 20> name;
};
struct SC_ROOM_PLAYERS_INFO {	// �濡 ���������� �����ϴ� �÷��̾���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomPlayersInfo;
	UINT roomID = 0;
	UINT hostID = 0;
	UINT nParticipant = 0;
	SC_SUB_ROOM_PLAYERS_INFO participantInfos[maxParticipant];
};
struct SC_GAME_START {	// ������ ������ ������ �� ���� ������ �������� Ȯ���Ͽ� ������
	SC_PACKET_TYPE type = SC_PACKET_TYPE::gameStart;
	UINT professorClientID;
};
struct SC_ROOM_VISIT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomVisitPlayerInfo;
	UINT visitClientID = 0;
	// std::array<char, 20> name;
};
struct SC_ROOM_OUT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomOutPlayerInfo;
	UINT outClientID = 0;
	UINT newHostID = 0;
};
struct SC_FAIL {		// �� ���忡 �������� ��� �����ִ� ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::fail;
	SC_FAIL_TYPE cause = SC_FAIL_TYPE::unknown;
};
struct SC_READY {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::ready;
	UINT readyClientID = 0;
};
struct SC_All_PLAYER_LOADING_COMPLETE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::allPlayerLoadingComplete;
};

struct SC_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::playerInfo;
	UINT clientID = 0;
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT3 size = XMFLOAT3(1, 1, 1);
	float aniTime = 0.0f;
};
struct SC_ANICLIP_CHANGE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::aniClipChange;
	UINT clientID = 0;
	std::array<char, 30> clipName;
};


#pragma pack(pop)