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
        ProcessRecv((SOCKET)_wParam);
        break;
    case FD_WRITE:  // https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=kkum04&logNo=150048096101
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
        RecvContents(_socket, packet);
        AddRoom(packet.hostID);
        break;
    }
    case CS_PACKET_TYPE::queryRoomlistInfo: {
        // ������ �ޱ�
        CS_QUERY_ROOMLIST_INFO recvPacket;
        RecvContents(_socket, recvPacket);

        // �� ����Ʈ�� ���� �����͸� �����.
        CreateRoomlistInfo();

        // �����͸� �����Ѵ�.
        send(_socket, buffer.data(), (int)buffer.size(), 0);
        break;
    }
    case CS_PACKET_TYPE::visitRoom: {
        // ������ �ޱ�
        CS_QUERY_VISIT_ROOM recvPacket;
        RecvContents(_socket, recvPacket);
        
        cout << recvPacket.cid << "�� Ŭ���̾�Ʈ - visitRoom ��Ŷ ���� : ";

        // ���� �������� ���� ���
        if (!pRooms.contains(recvPacket.visitRoomID)) { 
            cout << recvPacket.visitRoomID << "�� ���� �������� ����\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::noExistRoom;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // �ο��� �� á�� ���
        Room* pRoom = pRooms[recvPacket.visitRoomID];
        if (pRoom->GetNumOfParticipants() == maxParticipant ) {
            cout << recvPacket.visitRoomID << "�� ���� �ο��� ���� á��.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomOvercapacity;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // ������ ���۵Ǿ��� ���
        if (pRoom->IsGameRunning()) {
            cout << recvPacket.visitRoomID << "�� ���� ������ ���۵Ǿ���.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomGameStarted;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // ������ ������ ���
        // 1. �÷��̾ �����Ų �� ���� ������ �����Ѵ�.
        pRoom->EnterUser(recvPacket.cid);

        // 2. ������ ������ �ִ� �÷��̾� ���� ������ �����Ѵ�.
        SC_ROOM_VISIT_PLAYER_INFO sendPacket2;
        sendPacket2.visitClientID = recvPacket.cid;
        for (UINT clientID : pRoom->GetParticipants()) {
            if (clientID == recvPacket.cid)  // ������ �÷��̾��� ��� �����Ѵ�.
                continue;
            send(pClients[clientID]->GetSocket(), (char*)&sendPacket2, sizeof(SC_ROOM_VISIT_PLAYER_INFO), 0);
            cout << "������ ������ �ִ� �÷��̾� ���� ������ �����Ѵ�.\n";
        }
        break;
    }
    case CS_PACKET_TYPE::outRoom: {
        // ������ �ޱ�
        CS_OUT_ROOM recvPacket;
        RecvContents(_socket, recvPacket);

        // 1. �÷��̾ �濡�� ��������.
        Room* pRoom = pClients[recvPacket.cid]->GetCurrentRoom();
        int roomID = pRoom->GetID();
        pRoom->LeaveUser(recvPacket.cid);

        SendRoomOutPlayerAndRoomList(pRoom, pClients[recvPacket.cid]);

        //// 2. �濡 �����ϴ� �÷��̿��� ���� �÷��̾��� ������ �����Ѵ�.
        //if (pRooms.contains(roomID)) {
        //    SC_ROOM_OUT_PLAYER_INFO sendPacket;
        //    sendPacket.outClientID = recvPacket.cid;
        //    sendPacket.newHostID = pRoom->GetHostID();
        //    for (UINT clientID : pRoom->GetParticipants()) {
        //        cout << clientID << "�÷��̾�� " << sendPacket.outClientID << "�÷��̾ �����ٰ� ��Ŷ ������\n";
        //        send(pClients[clientID]->GetSocket(), (char*)&sendPacket, sizeof(SC_ROOM_OUT_PLAYER_INFO), 0);
        //    }
        //}

        //// 3. ���� �÷��̾�� ���� ����Ʈ ������ ������.
        //CreateRoomlistInfo();
        //send(_socket, buffer.data(), (int)buffer.size(), 0);

        break;
    }
    case CS_PACKET_TYPE::ready: {
        CS_READY recvPacket;
        RecvContents(_socket, recvPacket);

        Client* pClient = pClients[recvPacket.cid];
        Room* pRoom = pClient->GetCurrentRoom();
        // 1. ready��Ŷ�� ���� �÷��̾ ������ ���
        if (pRoom->GetHostID() == pClient->GetClientID()) {
            // 1-2. �ٸ� �÷��̾���� ��� �������� Ȯ���ϰ� ��� ������ ���¶�� ������ �����Ѵ�.
            
            if (pRoom->GetNumOfParticipants() >= 1) {
                bool success = true;
                for (UINT participant : pRoom->GetParticipants()) {
                    if (participant == pRoom->GetHostID())  // ������ �����ϰ�
                        continue;
                    if (pClients[participant]->GetClientState() != ClientState::roomReady) {
                        success = false;
                        break;
                    }
                }
                if (success) {  // ���ӽ���
                    pRoom->GameStart();
                }
                else {  // ���ۺҰ� - ������������ �ο��� �����Ѵ�.
                    SC_FAIL sendPacket;
                    sendPacket.cause = SC_FAIL_TYPE::notAllReady;
                    send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
                }
            }
            else {  // ���ۺҰ� - �ο�����
                SC_FAIL sendPacket;
                sendPacket.cause = SC_FAIL_TYPE::lackOfParticipants;
                send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            }
        }
        // 2. ready��Ŷ�� ���� �÷��̾ ������ �ƴ� ���
        else {
            if (pClient->GetClientState() == ClientState::roomWait)
                pClient->SetClientState(ClientState::roomReady);
            else if (pClient->GetClientState() == ClientState::roomReady)
                pClient->SetClientState(ClientState::roomWait);

            SC_READY sendPacket;
            sendPacket.readyClientID = pClient->GetClientID();
            //�ش��÷��̾ �����ߴٴ� ���� �˸���.
            for (UINT participant : pRoom->GetParticipants())
                send(pClients[participant]->GetSocket(), (char*)&sendPacket, sizeof(SC_READY), 0);
        }
        break;
    }
    case CS_PACKET_TYPE::loadingComplete: {
        CS_LOADING_COMPLETE recvPacket;
        RecvContents(_socket, recvPacket);

        cout << recvPacket.roomID << " �� �� �ε� �Ϸ� ! \n";
        PlayInfo* pPlayInfo = pPlayInfos[recvPacket.roomID];
        pPlayInfo->LoadingComplete(recvPacket.cid);
        break;
    }
    case CS_PACKET_TYPE::playerInfo: {
        CS_PLALYER_INFO recvPacket;
        RecvContents(_socket, recvPacket);

        // ���� ��Ŷ�� �����.
        SC_PLAYER_INFO sendPacket;
        sendPacket.aniTime = recvPacket.aniTime;
        sendPacket.clientID = recvPacket.cid;
        sendPacket.position = recvPacket.position;
        sendPacket.rotation = recvPacket.rotation;
        sendPacket.size = recvPacket.size;

        // �ٸ� �÷��̾�鿡�� ��Ŷ�� ������.
        PlayInfo* pPlayInfo = pClients[recvPacket.cid]->GetCurrentPlayInfo();
        for (UINT participant : pPlayInfo->GetParticipants()) {
            if (participant == recvPacket.cid)
                continue;
            send(pClients[participant]->GetSocket(), (char*)&sendPacket, sizeof(SC_PLAYER_INFO), 0);
        }

        break;
    }
    default:
        cout << (int)packetType << "�� �Դ�.\n";
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
        
        Client* pClient = pClients[_clientID];

        // Ŭ���̾�Ʈ�� �������̾��� ��� ǥ�ø� �صΰ� ������ ������ �����Ų��.
        PlayInfo* pPlayInfo = pClient->GetCurrentPlayInfo();
        if (pPlayInfo != NULL) {
            pClient->SetDisconnected(true);
        }

        // Ŭ���̾�Ʈ�� �濡 �־��� ��� ���� ������.
        Room* room = pClient->GetCurrentRoom();
        if (room != NULL) {
            room->LeaveUser(_clientID);
            SendRoomOutPlayerAndRoomList(room, pClient);
        }

        // �޸����� �� �����̳ʿ��� ����
        socketAndIdTable.erase(pClient->GetSocket());   // socket-id table���� �����Ѵ�.
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
    cout << roomIDCount << "�� ���� �߰��մϴ�.\n";
    Room* pNewRoom = new Room(roomIDCount);
    pRooms[roomIDCount] = pNewRoom;
    pNewRoom->EnterUser(hostID);

    ++roomIDCount;
}
bool ServerFramework::RemoveRoom(UINT roomID) {
    if (pRooms.contains(roomID) && pRooms[roomID]->GetNumOfParticipants() == 0) {    // �����ڰ� �Ѹ� ���� ���
        cout << roomID << "�� ���� �����մϴ�.\n";

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
        sendSubPacket.started = pRoom->IsGameRunning();
        buffer.insert(buffer.end(), (char*)&sendSubPacket, (char*)&sendSubPacket + sizeof(SC_SUB_ROOMLIST_INFO));
    }

}

void ServerFramework::AddPlayInfo(UINT _roomID) {

    PlayInfo* pPlayInfo = new PlayInfo(_roomID);
    pPlayInfos[_roomID] = pPlayInfo;
    pPlayInfo->Init(_roomID);

}

void ServerFramework::SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient) {
    // 2. �濡 �����ϴ� �÷��̿��� ���� �÷��̾��� ������ �����Ѵ�.
    SC_ROOM_OUT_PLAYER_INFO sendPacket;
    sendPacket.outClientID = pOutClient->GetClientID();
    sendPacket.newHostID = pRoom->GetHostID();
    for (UINT clientID : pRoom->GetParticipants()) {
        cout << clientID << "�÷��̾�� " << sendPacket.outClientID << "�÷��̾ �����ٰ� ��Ŷ ������\n";
        send(pClients[clientID]->GetSocket(), (char*)&sendPacket, sizeof(SC_ROOM_OUT_PLAYER_INFO), 0);
    }

    // 3. ���� �÷��̾�� ���� ����Ʈ ������ ������.
    CreateRoomlistInfo();
    send(pOutClient->GetSocket(), buffer.data(), (int)buffer.size(), 0);
}
