#include "stdafx.h"
#include "ServerFramework.h"

/// ���� ����
ServerFramework ServerFramework::instance;	// ���� �����ӿ�ũ 

/// ���� �Լ�
void ServerFramework::Init() {

}
void ServerFramework::Destroy() {
	
}
ServerFramework& ServerFramework::Instance() {
	return instance;
}

/// ������ �� �Ҹ���
ServerFramework::ServerFramework() {
    buffer.reserve(1024);
	clientIDCount = 1;
    roomIDCount = 1;
}
ServerFramework::~ServerFramework() {
}

///  �Ϲ��Լ�
void ServerFramework::ProcessSocketMessage(HWND _hWnd, UINT _message, WPARAM _wParam, LPARAM _lParam) { 
    /// wParam : ����, lParam : select�� �ʵ�
    
    // ������ ��ſ� ����� ����
    int result;

    // ���� �߻� ���� Ȯ��
    if ((int)WSAGETSELECTERROR(_lParam)) {
        SockErrorDisplay((int)WSAGETSELECTERROR(_lParam));
        RemoveClient(SocketToID((SOCKET)_wParam));   // �ش� ����(wParam)�� Ŭ���̾�Ʈ ����
        return;
    }

    // �޽��� ó��
    switch (WSAGETSELECTEVENT(_lParam)) {
        // ���ο� ������ ���� ��:
    case FD_ACCEPT: {

        // ���ο� Ŭ���̾�Ʈ�� ������ �޴´�.
        struct sockaddr_in clientaddr;
        int addrSize = sizeof(clientaddr);
        SOCKET clientSocket = accept(_wParam, (struct sockaddr*)&clientaddr, &addrSize);
        if (clientSocket == INVALID_SOCKET) {
            SockErrorDisplay("accept()");
            return;
        }
        cout << format("Ŭ���̾�Ʈ ���� : IP �ּ�={0}, ��Ʈ ��ȣ={1}\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ���ο� Ŭ���̾�Ʈ�� �߰��Ѵ�. (�����̳ʿ� �߰�)
        Client* pNewClient = new Client(clientSocket);
        AddClient(pNewClient);

        // ����Ʈ �𵨿� (�߰��� Ŭ���̾�Ʈ��)������ ���
        result = WSAAsyncSelect(clientSocket, _hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
        if (result == SOCKET_ERROR) {
            SockErrorDisplay("WSAAsyncSelect()");
            RemoveClient(clientIDCount);
        }

        // Ŭ���̾�Ʈ���� �ڽ��� �ο����� clientID�� �˷��ش�.
        SC_GIVE_CLIENT_ID sendPacket;
        sendPacket.clientID = pNewClient->GetClientID();
        send(clientSocket, (char*)&sendPacket, sizeof(SC_GIVE_CLIENT_ID), 0);
        break;
    }
    case FD_READ:
    case FD_WRITE:
        ProcessRecv((SOCKET)_wParam);
        break;
    case FD_CLOSE:
        RemoveClient(SocketToID((SOCKET)_wParam));
        break;
    }
}
void ServerFramework::ProcessRecv(SOCKET _socket) {
    CS_PACKET_TYPE packetType;
    recv(_socket, (char*)&packetType, sizeof(char), 0);

    switch (packetType) {
    case CS_PACKET_TYPE::makeRoom: {
        CS_MAKE_ROOM packet;
        recv(_socket, (char*)&packet + sizeof(CS_PACKET_TYPE), sizeof(CS_MAKE_ROOM) - sizeof(CS_PACKET_TYPE), 0);
        AddRoom(packet.hostID);
        break;
    }
    case CS_PACKET_TYPE::queryRoomlistInfo: {
        // ������ �ޱ�
        CS_QUERY_ROOMLIST_INFO recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_QUERY_ROOMLIST_INFO) - sizeof(CS_PACKET_TYPE), 0);

        // �� ����Ʈ�� ���� �����͸� �����.
        CreateRoomlistInfo();

        // �����͸� �����Ѵ�.
        send(_socket, buffer.data(), (int)buffer.size(), 0);
        break;
    }
    case CS_PACKET_TYPE::visitRoom: {
        // ������ �ޱ�
        CS_QUERY_VISIT_ROOM recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_QUERY_VISIT_ROOM) - sizeof(CS_PACKET_TYPE), 0);
        
        // ���� �������� ���� ���
        if (!pRooms.contains(recvPacket.visitRoomID)) { 
            SC_ROOM_VISIT_FAIL sendPacket;
            sendPacket.cause = 0;
            send(_socket, (char*)&sendPacket, sizeof(SC_ROOM_VISIT_FAIL), 0);
            break;
        }

        // �ο��� �� á�� ���
        Room* pRoom = pRooms[recvPacket.visitRoomID];
        if (pRoom->GetNumOfParticipants() == maxParticipant ) {
            SC_ROOM_VISIT_FAIL sendPacket;
            sendPacket.cause = 1;
            send(_socket, (char*)&sendPacket, sizeof(SC_ROOM_VISIT_FAIL), 0);
            break;
        }

        // ������ ������ ���
        // 1. �÷��̾ �����Ų��.
        pRoom->EnterUser(recvPacket.cid);
        // 2. �����ϴ� �÷��̾�� ������ �����Ѵ�.
        SC_ROOM_PLAYERS_INFO sendPacket1;
        sendPacket1.hostID = pRoom->GetHostID();
        sendPacket1.nParticipant = pRoom->GetNumOfParticipants();
        for (UINT i = 0; UINT clientID : pRoom->GetParticipants()) {
            sendPacket1.participantInfos[i].cid = clientID;
            sendPacket1.participantInfos[i].ready = pClients[clientID]->GetClientState() == ClientState::roomReady;
            ++i;
        }
        send(_socket, (char*)&sendPacket1, sizeof(SC_ROOM_PLAYERS_INFO), 0);
        // 3. ������ ������ �ִ� �÷��̾� ���� ������ �����Ѵ�.
        SC_ROOM_VISIT_PLAYER_INFO sendPacket2;
        sendPacket2.cid = recvPacket.cid;
        for (UINT clientID : pRoom->GetParticipants()) {
            if (clientID == recvPacket.cid)  // ������ �÷��̾��� ��� �����Ѵ�.
                continue;
            send(pClients[clientID]->GetSocket(), (char*)&sendPacket2, sizeof(SC_ROOM_VISIT_PLAYER_INFO), 0);
        }
        break;
    }
    case CS_PACKET_TYPE::outRoom: {
        // ������ �ޱ�
        CS_QUERY_OUT_ROOM recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_QUERY_OUT_ROOM) - sizeof(CS_PACKET_TYPE), 0);

        // 1. �÷��̾ �濡�� ��������.
        Room* pRoom = pClients[recvPacket.cid]->GetCurrentRoom();
        int roomID = pRoom->GetID();
        pRoom->LeaveUser(recvPacket.cid);

        // 2. �濡 �����ϴ� �÷��̿��� ���� �÷��̾��� ������ �����Ѵ�.
        if (pRooms.contains(roomID)) {
            SC_ROOM_OUT_PLAYER_INFO sendPacket;
            sendPacket.cid = recvPacket.cid;
            for (UINT clientID : pRoom->GetParticipants()) {
                send(pClients[clientID]->GetSocket(), (char*)&sendPacket, sizeof(SC_ROOM_OUT_PLAYER_INFO), 0);
            }
        }

        // 3. ���� �÷��̾�� ���� ����Ʈ ������ ������.
        CreateRoomlistInfo();
        send(_socket, buffer.data(), (int)buffer.size(), 0);

        break;
    }
    default:
        break;
    }


}

void ServerFramework::FrameAdvance() {
	// ������ �����Ű�鼭 �ʿ信 ���� �޽����� �����Ѵ�.
}
void ServerFramework::AddClient(Client* _pClient) {
	if (_pClient) {
        _pClient->SetClientID(clientIDCount);   // clientID�� �Ҵ��Ѵ�.
		pClients.emplace(clientIDCount, _pClient);  // client�� pClients �����̳ʿ� �߰��Ѵ�.
        socketAndIdTable[_pClient->GetSocket()] = clientIDCount;    // socket-id table�� �߰��Ѵ�.
        cout << format("-> Ŭ���̾�Ʈ ���� : clientID - {0}\n", _pClient->GetClientID());
        ++clientIDCount;    // clientIDCount�� ������Ų��.
	}
}
void ServerFramework::RemoveClient(UINT _clientID) {
    if (pClients.contains(_clientID)) {
        cout << format("<- Ŭ���̾�Ʈ ���� : clientID - {0}\n", pClients[_clientID]->GetClientID());
        
        // Ŭ���̾�Ʈ�� �濡 �־��� ��� ���� ������.
        ClientState clientState = pClients[_clientID]->GetClientState();
        if (clientState == ClientState::roomReady || clientState == ClientState::roomWait || clientState == ClientState::roomWaitComeback || clientState == ClientState::roomPlay) {
            Room* room = pClients[_clientID]->GetCurrentRoom();
            room->LeaveUser(_clientID);
        }
        socketAndIdTable.erase(pClients[_clientID]->GetSocket());   // socket-id table���� �����Ѵ�.
        delete pClients[_clientID];
        pClients.erase(_clientID);
    }
}
UINT ServerFramework::SocketToID(SOCKET _socket) {
    if (socketAndIdTable.contains(_socket))
        return socketAndIdTable[_socket];
    cout << "SocketToID(SOCKET _socket) : �������� �ʴ� socket-id table�� �����߽��ϴ�.\n";
    return -1;
}

void ServerFramework::AddRoom(UINT hostID) {
    Room* pNewRoom = new Room(roomIDCount);
    pRooms[roomIDCount] = pNewRoom;
    pNewRoom->EnterUser(hostID);

    ++roomIDCount;
}
bool ServerFramework::RemoveRoom(UINT roomID) {
    if (pRooms.contains(roomID) && pRooms[roomID]->GetNumOfParticipants() == 0) {    // �����ڰ� �Ѹ� ���� ���
        delete pRooms[roomID];
        pRooms.erase(roomID);
        return true;
    }
    return false;
}

void ServerFramework::CreateRoomlistInfo() {
    // ���� �����͸� �����.
    buffer.clear();

    SC_ROOMLIST_INFO sendPacket;
    sendPacket.nRoom = (UINT)pRooms.size();
    // 1. ���� ������ ���� ���� ����
    buffer.insert(buffer.end(), (char*)&sendPacket, (char*)&sendPacket + sizeof(SC_ROOMLIST_INFO));
    // 2. �� �濡 ���� ������ ����
    for (auto [roomID, pRoom] : pRooms) {
        SC_SUB_ROOMLIST_INFO sendSubPacket;
        sendSubPacket.nParticipant = pRoom->GetNumOfParticipants();
        sendSubPacket.roomID = roomID;
        buffer.insert(buffer.end(), (char*)&sendSubPacket, (char*)&sendSubPacket + sizeof(SC_SUB_ROOMLIST_INFO));
    }
}
