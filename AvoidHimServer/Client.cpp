#include "stdafx.h"
#include "Client.h"

Client::Client(SOCKET _socket) : clientID{ -1 }, socket{_socket} {
	state = ClientState::lobby;
	currentRoom = -1;
}
Client::~Client() {
	closesocket(socket);
}
