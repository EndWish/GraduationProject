#pragma once
// ���� ���� ��� ����

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define WM_SOCKET  (WM_USER+1)



///////////////////////////////

#pragma pack(push, 1)

struct TEST_PACKET {
	char type = 0;
	UINT cid;
	char letter;
};

#pragma pack(pop)