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

#define BUFSIZE 512
#define MAX_PARTICIPANT 5
#define SERVER_PERIOD (1.f / 20)
#define WATER_DISPENSER_COOLTIME 10.f
///////////////////////////////

enum class CS_PACKET_TYPE : char {
	none, makeRoom, queryRoomlistInfo, visitRoom, outRoom, ready, loadingComplete, playerInfo, aniClipChange,
	toggleDoor, useWaterDispenser, queryUseComputer, hackingRate, attack, hit, goPrison, openPrisonDoor,
	useItem, removeItem, toggleLever, removeTrap, exitPlayer, exitGame, checkNickname,
};

enum class SC_PACKET_TYPE : char {
	none, giveClientID, roomListInfo, roomPlayersInfo, roomVisitPlayerInfo, roomOutPlayerInfo, fail,
	ready, gameStart, allPlayerLoadingComplete, playerInfo, aniClipChange, yourPlayerObjectID,
	playersInfo, toggleDoor, useWaterDispenser, useComputer, hackingRate, attack, hit, goPrison, openPrisonDoor,
	addItem, useItem, removeItem, toggleLever, removeTrap, exitPlayer, professorWin, checkNickname, escapeProfessor, escapeStudent,

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
	otherPlayer,
	obstacle,
	attack,
	light,
	effect,
	sprite,
	item,
	trap,
	num,
	etc
};

enum class ShaderType : char {
	none,
	basic,
	instancing,
	blending,
	skinned,
	effect,
	num
};

enum class ObjectType : char {
	none,
	wall,
	Rdoor,
	Ldoor,
	lever,
	waterDispenser,
	studentStartPosition,
	professorStartPosition,
	computer,
	prisonDoor,
	light,
	exitRDoor,
	exitLDoor,
	prisonPosition,
	prisonExitPosition,
	prisonKeyItem,
	trapItem,
	medicalKitItem,
	energyDrinkItem,
	itemSpawnLocation,
	trap,
	exitBox,
};

enum class AttackType : char {
	none,
	swingAttack,
	throwAttack,
	num,
};


#pragma pack(push, 1)

// Ŭ�� -> ����
struct CS_MAKE_ROOM {	// ���� ����� �޶�� ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::makeRoom;
	UINT hostID = 0;
	UINT pid = 0;
};

struct CS_QUERY_ROOMLIST_INFO {	// �κ񿡼� �濡���� ������ �䱸�Ҷ� ������ ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::queryRoomlistInfo;
	UINT cid = 0;
	UINT roomPage = 0;
	UINT pid = 0;
};
struct CS_QUERY_VISIT_ROOM {	// �濡 �����Ҷ� �����ϴ� ��Ŷ
	CS_PACKET_TYPE type = CS_PACKET_TYPE::visitRoom;
	UINT cid = 0;
	UINT visitRoomID = 0;
	UINT pid = 0;
};
struct CS_OUT_ROOM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::outRoom;
	UINT cid = 0;
	UINT pid = 0;
};
struct CS_READY {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::ready;
	UINT cid = 0;
	UINT pid = 0;
};
struct CS_LOADING_COMPLETE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::loadingComplete;
	UINT cid = 0;
	UINT roomID = 0;
	UINT pid = 0;
};
struct CS_PLAYER_INFO {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::playerInfo;
	UINT cid = 0;
	UINT objectID = 0;
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	float aniTime = 0.0f;
	UINT pid = 0;
};
struct CS_ANICLIP_CHANGE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::aniClipChange;
	UINT cid = 0;
	std::array<char, 30> clipName;
	UINT pid = 0;
};
struct CS_TOGGLE_DOOR {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::toggleDoor;
	UINT cid = 0;
	UINT objectID = 0;
	UINT playerObjectID = 0;
	UINT pid = 0;
};
struct CS_USE_WATER_DISPENSER {	// �ش� �����⸦ ����ϰڴٰ� ����
	CS_PACKET_TYPE type = CS_PACKET_TYPE::useWaterDispenser;
	UINT cid = 0;
	UINT playerObjectID = 0;
	UINT objectID = 0;
	UINT pid = 0;
};
struct CS_QUERY_USE_COMPUTER {    // �ش� �����⸦ ����ϰڴٰ� ����
	CS_PACKET_TYPE type = CS_PACKET_TYPE::queryUseComputer;
	UINT cid = 0;
	UINT playerObjectID = 0;
	UINT computerObjectID = 0;
	UINT pid = 0;
};
struct CS_HACKING_RATE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::hackingRate;
	UINT cid = 0;
	UINT computerObjectID = 0;
	float rate = 0.f;
	UINT pid = 0;
};
struct CS_ATTACK {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::attack;
	UINT cid = 0;
	AttackType attackType = AttackType::none;
	UINT playerObjectID = 0;	// ������ ID
	UINT pid = 0;
};
struct CS_ATTACK_HIT {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::hit;
	UINT cid = 0;
	AttackType attackType = AttackType::none;
	UINT hitPlayerObjectID = 0;	// ���� �� ID
	UINT attackObjectID = 0;	// ���� ID
	UINT pid = 0;
};
struct CS_GO_PRISON {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::goPrison;
	UINT cid = 0;
	UINT playerObjectID = 0;	// ������ �÷��̾��� ������Ʈ ID
	UINT pid = 0;
};

struct CS_USE_ITEM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::useItem;
	UINT cid = 0;
	UINT playerObjectID = 0;
	ObjectType itemType = ObjectType::none;
	// ��Ʈ��ũ ���� �� ������ ���� Ʈ���� ��ġ�� �޶��� �� �����Ƿ� ��ġ�� ��Ȯ�� �����ش�.
	XMFLOAT3 usePosition = XMFLOAT3(0, 0, 0);
	UINT pid = 0;
};
struct CS_REMOVE_ITEM {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::removeItem;
	UINT cid = 0;
	UINT playerObjectID = 0;
	UINT itemObjectID = 0;
	UINT itemLocationIndex = 0;
	ObjectType itemType = ObjectType::none;
	UINT pid = 0;
};
struct CS_OPEN_PRISON_DOOR {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::openPrisonDoor;
	UINT cid = 0;
	UINT pid = 0;
};
struct CS_LEVER_TOGGLE {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::toggleLever;
	UINT cid = 0;
	UINT leverObjectID = 0;
	bool setPower = false;
	UINT pid = 0;
};

struct CS_REMOVE_TRAP {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::removeTrap;
	UINT cid = 0;
	UINT trapObjectID = 0;
	UINT pid = 0;
};
struct CS_EXIT_PLAYER {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::exitPlayer;
	UINT cid = 0;
	UINT playerObjectID = 0;
	UINT pid = 0;
};
struct CS_EXIT_GAME {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::exitGame;
	UINT cid = 0;
	UINT pid = 0;
};

struct CS_CHECK_EXIST_NICKNAME {
	CS_PACKET_TYPE type = CS_PACKET_TYPE::checkNickname;
	UINT cid = 0;
	WCHAR nickname[20];
	UINT pid = 0;
};

/// ����->Ŭ��
struct SC_GIVE_CLIENT_ID {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::giveClientID;
	UINT clientID = 0;
	UINT pid = 100'000;
};
struct SC_SUB_ROOMLIST_INFO {
	UINT roomID = 0;
	UINT nParticipant = 0;
	bool started = false;
	// std::array<char, 50> roomName;
	UINT pid = 100'000;
};
struct SC_ROOMLIST_INFO {	// �κ񿡼� ���� �ʿ��� ��鿡 ���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomListInfo;
	SC_SUB_ROOMLIST_INFO roomInfo[6];
	// nRoom ������ŭ "SC_SUB_ROOMLIST_INFO"�� �����Ѵ�.
	UINT pid = 100'000;
};
struct SC_SUB_ROOM_PLAYERS_INFO {
	UINT clientID = 0;
	bool ready = false;
	WCHAR name[20];
};
struct SC_ROOM_PLAYERS_INFO {	// �濡 ���������� �����ϴ� �÷��̾���� ������ ������ ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomPlayersInfo;
	UINT roomID = 0;
	UINT hostID = 0;
	UINT nParticipant = 0;
	SC_SUB_ROOM_PLAYERS_INFO participantInfos[MAX_PARTICIPANT];
	UINT pid = 100'000;
};
struct SC_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::playerInfo;
	UINT objectID = 0;
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	float aniTime = 0.0f;
	UINT pid = 100'000;
};
struct SC_PLAYERS_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::playersInfo;
	UINT nPlayer = 0;
	SC_PLAYER_INFO playersInfo[MAX_PARTICIPANT];
	UINT pid = 100'000;
};
struct SC_GAME_START {	// ������ ������ ������ �� ���� ������ �������� Ȯ���Ͽ� ������
	SC_PACKET_TYPE type = SC_PACKET_TYPE::gameStart;
	UINT professorObjectID = 0;
	UINT nPlayer = 0;
	SC_PLAYER_INFO playerInfo[MAX_PARTICIPANT];
	UINT activeComputerObjectID[MAX_PARTICIPANT]{ 0,0,0,0,0 };
	WCHAR nickname[MAX_PARTICIPANT][20];
	// SC_PLAYER_INFO �� nPlayer��ŭ �߰��� �����Ѵ�.
	UINT pid = 100'000;
};

struct SC_YOUR_PLAYER_OBJECTID {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::yourPlayerObjectID;
	UINT objectID = 0;
	UINT pid = 100'000;
};
struct SC_ROOM_VISIT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomVisitPlayerInfo;
	UINT visitClientID = 0;
	WCHAR name[20];
	UINT pid = 100'000;
};
struct SC_ROOM_OUT_PLAYER_INFO {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::roomOutPlayerInfo;
	UINT outClientID = 0;
	UINT newHostID = 0;
	UINT pid = 100'000;
};
struct SC_FAIL {		// �� ���忡 �������� ��� �����ִ� ��Ŷ
	SC_PACKET_TYPE type = SC_PACKET_TYPE::fail;
	SC_FAIL_TYPE cause = SC_FAIL_TYPE::unknown;
	UINT pid = 100'000;
};
struct SC_READY {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::ready;
	UINT readyClientID = 0;
	UINT pid = 100'000;
};
struct SC_All_PLAYER_LOADING_COMPLETE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::allPlayerLoadingComplete;
	UINT pid = 100'000;
};
struct SC_ANICLIP_CHANGE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::aniClipChange;
	UINT clientID = 0;
	std::array<char, 30> clipName;
	UINT pid = 100'000;
};
struct SC_TOGGLE_DOOR {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::toggleDoor;
	UINT objectID = 0;
	UINT pid = 100'000;
};
struct SC_USE_WATER_DISPENSER {	// �ش� �����⸦ cid �÷��̾ ����ߴٰ� ����
	SC_PACKET_TYPE type = SC_PACKET_TYPE::useWaterDispenser;
	UINT playerObjectID = 0;
	UINT waterDispenserObjectID = 0;
	UINT pid = 100'000;
};
struct SC_USE_COMPUTER {	// �ش� �����⸦ cid �÷��̾ ����ߴٰ� ����
	SC_PACKET_TYPE type = SC_PACKET_TYPE::useComputer;
	UINT playerObjectID = 0;
	UINT computerObjectID = 0;
	UINT pid = 100'000;
};
struct SC_HACKING_RATE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::hackingRate;
	UINT computerObjectID = 0;
	float rate = 0.f;
	UINT pid = 100'000;
};

struct SC_ATTACK {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::attack;
	UINT attackObjectID = 0;		// ���� ID
	AttackType attackType = AttackType::none;
	UINT playerObjectID = 0;		// ������ ID
	UINT pid = 100'000;
};
struct SC_ATTACK_HIT {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::hit;
	AttackType attackType = AttackType::none;
	UINT hitPlayerObjectID = 0;		// ���� �� ID
	UINT attackObjectID = 0;		// ���� ID
	UINT pid = 100'000;
};
struct SC_GO_PRISON {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::goPrison;
	UINT playerObjectID = 0;	// ���� �÷��̾��� ������Ʈ ID
	UINT pid = 100'000;
};
struct SC_OPEN_PRISON_DOOR {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::openPrisonDoor;
	UINT openPlayerObjectID = 0;
	UINT pid = 100'000;
};

struct SC_ADD_ITEM {	// Ư�� ������ �������� ���� ���
	SC_PACKET_TYPE type = SC_PACKET_TYPE::addItem;
	UINT itemLocationIndex = 0;
	UINT itemObjectID = 0;
	ObjectType objectType = ObjectType::none;
	UINT pid = 100'000;
};

struct SC_REMOVE_ITEM {	// Ư�� �÷��̾ �������� ȹ���Ͽ� �� �ڸ� �������� ���� ���
	SC_PACKET_TYPE type = SC_PACKET_TYPE::removeItem;
	UINT itemObjectID = 0;
	UINT pid = 100'000;
};

struct SC_USE_ITEM {	// Ư�� �÷��̾ �������� ����� ���
	SC_PACKET_TYPE type = SC_PACKET_TYPE::useItem;
	UINT playerObjectID = 0;
	ObjectType objectType = ObjectType::none;
	UINT itemObjectID = 0;	// ������Ʈ�� �߰��Ǿ� �� ��츸 ���.
	UINT pid = 100'000;

};
struct SC_LEVER_TOGGLE {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::toggleLever;
	UINT leverObjectID = 0;
	bool setPower = false;
	bool allLeverPowerOn = false;
	UINT pid = 100'000;
};
struct SC_REMOVE_TRAP {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::removeTrap;
	UINT trapObjectID = 0;
	UINT pid = 100'000;
};
struct SC_EXIT_PLAYER {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::exitPlayer;
	UINT playerObjectID = 0;
	UINT pid = 100'000;
};
struct SC_CHECK_NICKNAME {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::checkNickname;
	bool isExist = false;
	UINT pid = 100'000;
};
struct SC_PROFESSOR_WIN {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::professorWin;
	UINT pid = 100'000;
};
struct SC_ESCAPE_PROFESSOR {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::escapeProfessor;
	UINT pid = 100'000;
};
struct SC_ESCAPE_STUDENT {
	SC_PACKET_TYPE type = SC_PACKET_TYPE::escapeStudent;
	UINT escapeObjectID = 0;
	UINT pid = 100'000;
};

#pragma pack(pop)