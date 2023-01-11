#include "stdafx.h"
#include "Client.h"
#include "Room.h"

Client::Client(SOCKET _socket) : socket{_socket} {
	clientID = 0;
	state = ClientState::lobby;
	pCurrentRoom = NULL;
	pGamePlayInfo = NULL;
}
Client::~Client() {
	closesocket(socket);
}



///////////////////////////////////////////////////////////////////////////////
/// ClientPlayInfo

ClientPlayInfo::ClientPlayInfo() {
	isProfessor = false;
	hp = 0;
	moveSpeed = 0;
	jumpSpeed = 0;
	chanceUnlockLab = 0;
}
ClientPlayInfo::~ClientPlayInfo() {

}

void ClientPlayInfo::SetProfessor() {
	isProfessor = true;
	hp = 100;
	moveSpeed = 1.0;
	jumpSpeed = 0;
	chanceUnlockLab = 0;
}

void ClientPlayInfo::SetStudent() {
	isProfessor = false;
	hp = 100;
	moveSpeed = 1.23f;
	jumpSpeed = 2.0f;
	chanceUnlockLab = 1;
}

