#include "stdafx.h"
#include "ServerFramework.h"

/// 전역 변수
ServerFramework ServerFramework::instance;	// 고유 프레임워크 

/// 전역 함수
void ServerFramework::Init() {

}
void ServerFramework::Destroy() {
	
}
ServerFramework& ServerFramework::Instance() {
	return instance;
}

/// 생성자 및 소멸자
ServerFramework::ServerFramework() {
    buffer.reserve(1024);
	clientIDCount = 1;
    roomIDCount = 1;
    playInfoIDCount = 1;
}
ServerFramework::~ServerFramework() {
}

///  일반함수
void ServerFramework::ProcessSocketMessage(HWND _hWnd, UINT _message, WPARAM _wParam, LPARAM _lParam) { 
    /// wParam : 소켓, lParam : select의 필드
    
    // 데이터 통신에 사용할 변수
    int result;

    // 오류 발생 여부 확인
    if ((int)WSAGETSELECTERROR(_lParam)) {
        SockErrorDisplay((int)WSAGETSELECTERROR(_lParam));
        RemoveClient(SocketToID((SOCKET)_wParam));   // 해당 소켓(wParam)의 클라이언트 제거
        return;
    }

    // 메시지 처리
    switch (WSAGETSELECTEVENT(_lParam)) {
        // 새로운 접속이 왔을 때:
    case FD_ACCEPT: {

        // 새로운 클라이언트의 접속을 받는다.
        struct sockaddr_in clientaddr;
        int addrSize = sizeof(clientaddr);
        SOCKET clientSocket = accept(_wParam, (struct sockaddr*)&clientaddr, &addrSize);
        if (clientSocket == INVALID_SOCKET) {
            SockErrorDisplay("accept()");
            return;
        }
        cout << format("클라이언트 접속 : IP 주소={0}, 포트 번호={1}\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 새로운 클라이언트를 추가한다. (컨테이너에 추가)
        Client* pNewClient = new Client(clientSocket);
        AddClient(pNewClient);

        // 셀렉트 모델에 (추가된 클라이언트의)소켓을 등록
        result = WSAAsyncSelect(clientSocket, _hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
        if (result == SOCKET_ERROR) {
            SockErrorDisplay("WSAAsyncSelect()");
            RemoveClient(clientIDCount);
        }

        // 클라이언트에게 자신이 부여받은 clientID를 알려준다.
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
        recv(_socket, (char*)&packet + sizeof(CS_PACKET_TYPE), sizeof(CS_MAKE_ROOM) - sizeof(CS_PACKET_TYPE), 0);
        AddRoom(packet.hostID);
        break;
    }
    case CS_PACKET_TYPE::queryRoomlistInfo: {
        // 데이터 받기
        CS_QUERY_ROOMLIST_INFO recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_QUERY_ROOMLIST_INFO) - sizeof(CS_PACKET_TYPE), 0);

        // 방 리스트에 대한 데이터를 만든다.
        CreateRoomlistInfo();

        // 데이터를 전송한다.
        send(_socket, buffer.data(), (int)buffer.size(), 0);
        break;
    }
    case CS_PACKET_TYPE::visitRoom: {
        // 데이터 받기
        CS_QUERY_VISIT_ROOM recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_QUERY_VISIT_ROOM) - sizeof(CS_PACKET_TYPE), 0);
        
        cout << recvPacket.cid << "번 클라이언트 - visitRoom 패킷 받음 : ";

        // 방이 존재하지 않은 경우
        if (!pRooms.contains(recvPacket.visitRoomID)) { 
            cout << recvPacket.visitRoomID << "번 방은 존재하지 않음\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::noExistRoom;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // 인원이 꽉 찼을 경우
        Room* pRoom = pRooms[recvPacket.visitRoomID];
        if (pRoom->GetNumOfParticipants() == maxParticipant ) {
            cout << recvPacket.visitRoomID << "번 방은 인원이 가득 찼다.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomOvercapacity;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // 게임이 시작되었을 경우
        if (pRoom->IsGameRunning()) {
            cout << recvPacket.visitRoomID << "번 방은 게임이 시작되었다.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomGameStarted;
            send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            break;
        }

        // 입장이 가능한 경우
        // 1. 플레이어를 입장시킨다.
        pRoom->EnterUser(recvPacket.cid);
        cout << recvPacket.visitRoomID << "번 방에 접속 성공.\n";
        // 2. 입장하는 플레이어에게 정보를 전송한다.
        SC_ROOM_PLAYERS_INFO sendPacket1;
        sendPacket1.hostID = pRoom->GetHostID();
        sendPacket1.nParticipant = pRoom->GetNumOfParticipants();
        for (UINT i = 0; UINT clientID : pRoom->GetParticipants()) {
            sendPacket1.participantInfos[i].clientID = clientID;
            sendPacket1.participantInfos[i].ready = pClients[clientID]->GetClientState() == ClientState::roomReady;
            ++i;
        }
        send(_socket, (char*)&sendPacket1, sizeof(SC_ROOM_PLAYERS_INFO), 0);
        cout << "입장하는 플레이어에게 정보를 전송한다.\n";
        // 3. 기존에 접속해 있는 플레이어 에게 정보를 전송한다.
        SC_ROOM_VISIT_PLAYER_INFO sendPacket2;
        sendPacket2.visitClientID = recvPacket.cid;
        for (UINT clientID : pRoom->GetParticipants()) {
            if (clientID == recvPacket.cid)  // 입장한 플레이어의 경우 제외한다.
                continue;
            send(pClients[clientID]->GetSocket(), (char*)&sendPacket2, sizeof(SC_ROOM_VISIT_PLAYER_INFO), 0);
            cout << "기존에 접속해 있는 플레이어 에게 정보를 전송한다.\n";
        }
        break;
    }
    case CS_PACKET_TYPE::outRoom: {
        // 데이터 받기
        CS_OUT_ROOM recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_OUT_ROOM) - sizeof(CS_PACKET_TYPE), 0);

        // 1. 플레이어를 방에서 내보낸다.
        Room* pRoom = pClients[recvPacket.cid]->GetCurrentRoom();
        int roomID = pRoom->GetID();
        pRoom->LeaveUser(recvPacket.cid);

        SendRoomOutPlayerAndRoomList(pRoom, pClients[recvPacket.cid]);

        //// 2. 방에 존재하는 플레이에게 나간 플레이어의 정보를 전송한다.
        //if (pRooms.contains(roomID)) {
        //    SC_ROOM_OUT_PLAYER_INFO sendPacket;
        //    sendPacket.outClientID = recvPacket.cid;
        //    sendPacket.newHostID = pRoom->GetHostID();
        //    for (UINT clientID : pRoom->GetParticipants()) {
        //        cout << clientID << "플레이어에게 " << sendPacket.outClientID << "플레이어가 나갔다고 패킷 전송함\n";
        //        send(pClients[clientID]->GetSocket(), (char*)&sendPacket, sizeof(SC_ROOM_OUT_PLAYER_INFO), 0);
        //    }
        //}

        //// 3. 나간 플레이어에게 방의 리스트 정보를 보낸다.
        //CreateRoomlistInfo();
        //send(_socket, buffer.data(), (int)buffer.size(), 0);

        break;
    }
    case CS_PACKET_TYPE::ready: {
        CS_READY recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_READY) - sizeof(CS_PACKET_TYPE), 0);

        Client* pClient = pClients[recvPacket.cid];
        Room* pRoom = pClient->GetCurrentRoom();
        // 1. ready패킷을 보낸 플레이어가 방장일 경우
        if (pRoom->GetHostID() == pClient->GetClientID()) {
            // 1-2. 다른 플레이어들이 모두 레디인지 확인하고 모두 레디한 상태라면 게임을 시작한다.
            
            if (pRoom->GetNumOfParticipants() >= 3) {
                bool success = true;
                for (UINT participant : pRoom->GetParticipants()) {
                    if (participant == pRoom->GetHostID())  // 방장은 제외하고
                        continue;
                    if (pClients[participant]->GetClientState() != ClientState::roomReady) {
                        success = false;
                        break;
                    }
                }
                if (success) {  // 게임시작
                    pRoom->GameStart();
                }
                else {  // 시작불가 - 레디하지않은 인원이 존재한다.
                    SC_FAIL sendPacket;
                    sendPacket.cause = SC_FAIL_TYPE::notAllReady;
                    send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
                }
            }
            else {  // 시작불가 - 인원부족
                SC_FAIL sendPacket;
                sendPacket.cause = SC_FAIL_TYPE::lackOfParticipants;
                send(_socket, (char*)&sendPacket, sizeof(SC_FAIL), 0);
            }
        }
        // 2. ready패킷을 보낸 플레이어가 방장이 아닐 경우
        else {
            if (pClient->GetClientState() == ClientState::roomWait)
                pClient->SetClientState(ClientState::roomReady);
            else if (pClient->GetClientState() == ClientState::roomReady)
                pClient->SetClientState(ClientState::roomWait);

            SC_READY sendPacket;
            sendPacket.readyClientID = pClient->GetClientID();
            //해당플레이어가 레디했다는 것을 알린다.
            for (UINT participant : pRoom->GetParticipants())
                send(pClients[participant]->GetSocket(), (char*)&sendPacket, sizeof(SC_READY), 0);
        }
        break;
    }
    case CS_PACKET_TYPE::loadingComplete: {
        CS_LOADING_COMPLETE recvPacket;
        recv(_socket, (char*)&recvPacket + sizeof(CS_PACKET_TYPE), sizeof(CS_LOADING_COMPLETE) - sizeof(CS_PACKET_TYPE), 0);

        PlayInfo* pPlayInfo = pPlayInfos[recvPacket.cid];
        pPlayInfo->LoadingComplete(recvPacket.cid);
        break;
    }
    default:
        cout << (int)packetType << "이 왔다.\n";
        break;
    }
}

void ServerFramework::FrameAdvance() {
	// 게임을 진행시키면서 필요에 따라 메시지를 전송한다.
    
}
void ServerFramework::AddClient(Client* _pClient) {
	if (_pClient) {
        _pClient->SetClientID(clientIDCount);   // clientID를 할당한다.
		pClients.emplace(clientIDCount, _pClient);  // client를 pClients 컨테이너에 추가한다.
        socketAndIdTable[_pClient->GetSocket()] = clientIDCount;    // socket-id table에 추가한다.
        cout << format("-> 클라이언트 접속 : clientID - {0}\n", _pClient->GetClientID());
        ++clientIDCount;    // clientIDCount를 증가시킨다.
	}
}
void ServerFramework::RemoveClient(UINT _clientID) {
    if (pClients.contains(_clientID)) {
        cout << format("<- 클라이언트 종료 : clientID - {0}\n", pClients[_clientID]->GetClientID());
        
        Client* pClient = pClients[_clientID];

        // 클라이언트가 게임중이었을 경우 표시를 해두고 게임이 끝나면 종료시킨다.
        PlayInfo* pPlayInfo = pClient->GetCurrentPlayInfo();
        if (pPlayInfo != NULL) {
            pClient->SetDisconnected(true);
        }

        // 클라이언트가 방에 있었을 경우 방을 나간다.
        Room* room = pClient->GetCurrentRoom();
        if (room != NULL) {
            room->LeaveUser(_clientID);
            SendRoomOutPlayerAndRoomList(room, pClient);
        }

        // 메모리해제 및 컨테이너에서 삭제
        socketAndIdTable.erase(pClient->GetSocket());   // socket-id table에서 제거한다.
        delete pClients[_clientID];
        pClients.erase(_clientID);
    }
}
UINT ServerFramework::SocketToID(SOCKET _socket) {
    if (socketAndIdTable.contains(_socket))
        return socketAndIdTable[_socket];
    cout << "SocketToID(SOCKET _socket) : 존재하지 않는 socket-id table에 접근했습니다.\n";
    return -1;
}

void ServerFramework::AddRoom(UINT hostID) {
    cout << roomIDCount << "번 방을 추가합니다.\n";
    Room* pNewRoom = new Room(roomIDCount);
    pRooms[roomIDCount] = pNewRoom;
    pNewRoom->EnterUser(hostID);

    ++roomIDCount;
}
bool ServerFramework::RemoveRoom(UINT roomID) {
    if (pRooms.contains(roomID) && pRooms[roomID]->GetNumOfParticipants() == 0) {    // 참가자가 한명도 없을 경우
        cout << roomID << "번 방을 삭제합니다.\n";

        delete pRooms[roomID];
        pRooms.erase(roomID);
        return true;
    }
    return false;
}

void ServerFramework::CreateRoomlistInfo() {
    // 보낼 데이터를 만든다.
    buffer.clear();

    SC_ROOMLIST_INFO sendPacket;
    sendPacket.nRoom = (UINT)pRooms.size();
    // 1. 방의 개수에 대한 정보 삽입
    buffer.insert(buffer.end(), (char*)&sendPacket, (char*)&sendPacket + sizeof(SC_ROOMLIST_INFO));
    // 2. 각 방에 대한 정보를 삽입
    for (auto [roomID, pRoom] : pRooms) {
        SC_SUB_ROOMLIST_INFO sendSubPacket;
        sendSubPacket.nParticipant = pRoom->GetNumOfParticipants();
        sendSubPacket.roomID = roomID;
        sendSubPacket.started = pRoom->IsGameRunning();
        buffer.insert(buffer.end(), (char*)&sendSubPacket, (char*)&sendSubPacket + sizeof(SC_SUB_ROOMLIST_INFO));
    }

}

void ServerFramework::AddPlayInfo(UINT _roomID) {
    PlayInfo* pPlayInfo = new PlayInfo(playInfoIDCount);
    pPlayInfos[playInfoIDCount] = pPlayInfo;
    pPlayInfo->Init(_roomID);

    ++playInfoIDCount;
}

void ServerFramework::SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient) {
    // 2. 방에 존재하는 플레이에게 나간 플레이어의 정보를 전송한다.
    SC_ROOM_OUT_PLAYER_INFO sendPacket;
    sendPacket.outClientID = pOutClient->GetClientID();
    sendPacket.newHostID = pRoom->GetHostID();
    for (UINT clientID : pRoom->GetParticipants()) {
        cout << clientID << "플레이어에게 " << sendPacket.outClientID << "플레이어가 나갔다고 패킷 전송함\n";
        send(pClients[clientID]->GetSocket(), (char*)&sendPacket, sizeof(SC_ROOM_OUT_PLAYER_INFO), 0);
    }

    // 3. 나간 플레이어에게 방의 리스트 정보를 보낸다.
    CreateRoomlistInfo();
    send(pOutClient->GetSocket(), buffer.data(), (int)buffer.size(), 0);
}
