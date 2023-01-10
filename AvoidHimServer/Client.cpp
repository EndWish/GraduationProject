#include "stdafx.h"
#include "Client.h"
#include "Room.h"

Client::Client(SOCKET _socket) : clientID{ 0U }, socket{_socket} {
	state = ClientState::lobby;
	pCurrentRoom = NULL;
}
Client::~Client() {
	closesocket(socket);
}
