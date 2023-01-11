#pragma once
// 서버 관련 헤더 파일

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define WM_SOCKET  (WM_USER+1)

const int maxParticipant = 5;

///////////////////////////////

enum class CS_PACKET_TYPE : char {
	makeRoom, queryRoomlistInfo, visitRoom, outRoom, ready, unready, loadComplete
};

enum class SC_PACKET_TYPE : char {
	giveClientID, roomListInfo, roomPlayersInfo, roomVisitPlayerInfo, roomOutPlayerInfo, fail, 
	ready, unready, 

};

enum class SC_FAIL_TYPE : int {
	unknown,	//  초기값
	noExistRoom,	// 방이 존재하지 않을때
	roomOvercapacity,	// 방에 정원이 초과했을때
	lackOfParticipants,	// 방에 참가자가 부족할때
	notAllReady,	// 준비하지 않은 인원이 존재할때
	roomGameStarted,	// 이미 게임이 시작되었을 경우
};

#pragma pack(push, 1)

// 클라 -> 서버
struct CS_MAKE_ROOM {	// 방을 만들어 달라는 패킷
	CS_PACKET_TYPE type = CS_PACKET_TYPE::makeRoom;
	UINT hostID = 0;
};
struct CS_QUERY_ROOMLIST_INFO {	// 로비에서 방에대한 정보를 요구할때 보내는 패킷
	CS_PACKET_TYPE type = CS_PACKET_TYPE::queryRoomlistInfo;
	UINT cid = 0;
};
struct CS_QUERY_VISIT_ROOM {	// 방에 입장할때 전송하는 패킷
	CS_PACKET_TYPE type = CS_PACKET_TYPE::visitRoom;
	UINT cid = 0;
	UINT visitRoomID = 0;
};
struct CS_QUERY_OUT_ROOM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::outRoom;
	UINT cid = 0;
};
struct CS_READY {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::ready;
	UINT cid = 0;
};
struct CS_UNREADY {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::unready;
	UINT cid = 0;
};
struct CS_LOAD_COMPLETE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::loadComplete;
	UINT cid = 0;
};

/// 서버->클라
struct SC_GIVE_CLIENT_ID {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::giveClientID;
	UINT clientID = 0;
};
struct SC_SUB_ROOMLIST_INFO {
	UINT roomID = 0;
	UINT nParticipant = 0;
	bool started = false;
	// array<char, 50> roomName;
};
struct SC_ROOMLIST_INFO {	// 로비에서 볼때 필요한 방들에 대한 정보를 보내는 패킷
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomListInfo;
	UINT nRoom = 0;
	// nRoom 개수만큼 "SC_SUB_ROOMLIST_INFO"를 전송한다.
};

struct SC_SUB_ROOM_PLAYERS_INFO {
	UINT cid = 0;
	bool ready = false;
	// array<char, 20> name;
};
struct SC_ROOM_PLAYERS_INFO {	// 방에 입장했을때 존재하는 플레이어들의 정보를 보내는 패킷
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

struct SC_FAIL {		// 방 입장에 실패했을 경우 보내주는 패킷
	SC_PACKET_TYPE type = SC_PACKET_TYPE::fail;
	SC_FAIL_TYPE cause = SC_FAIL_TYPE::unknown;
};

struct SC_READY {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::ready;
	UINT readyClientID = 0;
};
struct SC_UNREADY {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::unready;
	UINT unreadyClientID = 0;
};

#pragma pack(pop)