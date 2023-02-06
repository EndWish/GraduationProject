#include "stdafx.h"
#include "Client.h"
#include "Room.h"

Client::Client(SOCKET _socket) : socket{_socket} {
	clientID = 0;
	state = ClientState::lobby;
	pCurrentRoom = NULL;
	pCurrentPlayInfo = NULL;
	disconnected = false;
	lastPacketType = CS_PACKET_TYPE::none;
}
Client::~Client() {
	closesocket(socket);
}

