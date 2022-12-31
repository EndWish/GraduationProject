#include "stdafx.h"
#include "Client.h"

Client::Client(int _clientID, SOCKET _socket) : clientID{ _clientID }, socket{_socket} {

}
Client::~Client() {
	closesocket(socket);
}
