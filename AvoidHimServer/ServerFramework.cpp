#include "stdafx.h"
#include "ServerFramework.h"
#include "GameObject.h"

/// 전역 변수
ServerFramework ServerFramework::instance;	// 고유 프레임워크 

/// 전역 함수
void ServerFramework::Init(HWND _windowHandle) {
    LoadMapFile();
    windowHandle = _windowHandle;
}
void ServerFramework::Destroy() {
    for (auto [key, pClient] : pClients)
        delete pClient;
    for (auto [key, pRoom] : pRooms)
        delete pRoom;
    for (auto [key, pPlayInfo] : pPlayInfos)
        delete pPlayInfo;

    for (auto [key, pInitialObject] : pInitialObjects)
        delete pInitialObject;
}
ServerFramework& ServerFramework::Instance() {
	return instance;
}

/// 생성자 및 소멸자
ServerFramework::ServerFramework() {
    windowHandle = HWND();


	clientIDCount = 1;
    roomIDCount = 1;
    professorStartPosition = XMFLOAT3(0, 0, 0);
    itemSpawnLocationCount = 0;
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

        //int optVal = 300 * 50;
        //setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, sizeof(optVal));

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
        SendContents(pNewClient->GetSocket(), pNewClient->GetRemainBuffer(), sendPacket);
        break;
    }
    case FD_READ: {
        ProcessRecv((SOCKET)_wParam);
        break;
    }
    case FD_WRITE: {  // https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=kkum04&logNo=150048096101 
        auto& remainBuffer = pClients[socketAndIdTable[(SOCKET)_wParam]]->GetRemainBuffer();
        if (remainBuffer[0] != 0)
            cout << "FD_WRITE - send Byte : " << send((SOCKET)_wParam, remainBuffer.data(), BUFSIZE, 0) << "\n";
        break;
    }

    case FD_CLOSE: {
        RemoveClient(SocketToID((SOCKET)_wParam));
        break;
    }
        
    }
}
void ServerFramework::ProcessRecv(SOCKET _socket) {
    Client* pClient = nullptr;
    if (socketAndIdTable.contains(_socket)) {
        pClient = pClients[socketAndIdTable[_socket]];
    }
    else {
        pClient = nullptr;
        return;
    }

    int& recvByte = pClient->GetRecvByte();
    auto& recvBuffer = pClient->GetRecvBuffer();
    int result = recv(_socket, recvBuffer.data() + recvByte, BUFSIZE - recvByte, 0);
    if (result == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            cout << "WARNING : WSAEWOULDBLOCK\n";
        else
            SockErrorDisplay("recv() : ");
        return;
    }
    else {
        recvByte += result;

        if (recvByte < BUFSIZE) {
            return;
        }
        else {
            recvByte = 0;
        }
    }
    globalRecvBuffer = recvBuffer;

    // 패킷타입을 읽는다.
    CS_PACKET_TYPE& packetType = GetPacket<CS_PACKET_TYPE>();

    memcpy((char*)&packetType, globalRecvBuffer.data(), sizeof(packetType));

    switch (packetType) {
    case CS_PACKET_TYPE::checkNickname: {
        // 닉네임 중복 체크를 하고 중복이 없을경우 그 아이디를 저장한다.
        SC_CHECK_NICKNAME sendPacket;
        sendPacket.isExist = false;
        CS_CHECK_EXIST_NICKNAME& recvPacket = GetPacket<CS_CHECK_EXIST_NICKNAME>();
        for (auto [cid, pCheckClient] : pClients) {
            if (pClients[recvPacket.cid] == pCheckClient) continue;
            if (wcscmp(pCheckClient->GetNickname().c_str(), recvPacket.nickname) == 0) {
                // 닉네임이 같은 경우
                sendPacket.isExist = true;

                cout << pClients.size();
                break;
            }
        }
        SendContents(_socket, pClient->GetRemainBuffer(), sendPacket);
        if (!sendPacket.isExist) {
            pClients[recvPacket.cid]->SetNickname(wstring(recvPacket.nickname));

        }
        else {
            // 중복되는 이름일 경우 해당 클라이언트를 바로 삭제해준다.
            pClients.erase(recvPacket.cid);
            delete pClient;
        }

        
        break;
    }

    case CS_PACKET_TYPE::makeRoom: {
        CS_MAKE_ROOM& recvPacket = GetPacket<CS_MAKE_ROOM>();

        cout << format("CS_MAKE_ROOM : hostID - {}, pid - {} \n", recvPacket.hostID, recvPacket.pid);

        AddRoom(recvPacket.hostID);
        break;
    }
    case CS_PACKET_TYPE::queryRoomlistInfo: {
        // 데이터 받기
        CS_QUERY_ROOMLIST_INFO& recvPacket = GetPacket<CS_QUERY_ROOMLIST_INFO>();
        cout << format("CS_QUERY_ROOMLIST_INFO : cid - {}, roomPage - {}, pid - {}  \n", recvPacket.cid, recvPacket.roomPage, recvPacket.pid);

        // 방 리스트에 대한 데이터를 만든다.
        SendRoomlistInfo(pClient, recvPacket.roomPage);
        break;
    }
    case CS_PACKET_TYPE::visitRoom: {
        // 데이터 받기
        CS_QUERY_VISIT_ROOM& recvPacket = GetPacket<CS_QUERY_VISIT_ROOM>();
        cout << format("CS_QUERY_VISIT_ROOM : cid - {}, visitRoomID - {}, pid - {}  \n", recvPacket.cid, recvPacket.visitRoomID, recvPacket.pid);

        // 방이 존재하지 않은 경우
        if (!pRooms.contains(recvPacket.visitRoomID)) { 
            cout << recvPacket.visitRoomID << "번 방은 존재하지 않음\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::noExistRoom;
            SendContents(_socket, pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // 인원이 꽉 찼을 경우
        Room* pRoom = pRooms[recvPacket.visitRoomID];
        if (pRoom->GetNumOfParticipants() == MAX_PARTICIPANT ) {
            cout << recvPacket.visitRoomID << "번 방은 인원이 가득 찼다.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomOvercapacity;
            SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // 게임이 시작되었을 경우
        if (pRoom->IsGameRunning()) {
            cout << recvPacket.visitRoomID << "번 방은 게임이 시작되었다.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomGameStarted;
            SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // 입장이 가능한 경우
        // 1. 플레이어를 입장시킨 후 방의 정보를 전송한다.
        pRoom->EnterUser(recvPacket.cid);

        // 2. 기존에 접속해 있는 플레이어 에게 정보를 전송한다.
        SC_ROOM_VISIT_PLAYER_INFO sendPacket;
        memcpy(sendPacket.name, pClients[recvPacket.cid]->GetNickname().c_str(), 20);
        sendPacket.visitClientID = recvPacket.cid;
        for (UINT clientID : pRoom->GetParticipants()) {
            if (clientID == recvPacket.cid)  // 입장한 플레이어의 경우 제외한다.
                continue;
            SendContents(pClients[clientID]->GetSocket(), pClients[clientID]->GetRemainBuffer(), sendPacket);
            cout << "기존에 접속해 있는 플레이어 에게 정보를 전송한다.\n";
        }
        break;
    }
    case CS_PACKET_TYPE::outRoom: {
        // 데이터 받기
        CS_OUT_ROOM& recvPacket = GetPacket<CS_OUT_ROOM>();
        cout << format("CS_OUT_ROOM : cid - {}, pid - {} \n", recvPacket.cid, recvPacket.pid);

        // 1. 플레이어를 방에서 내보낸다.
        Room* pRoom = pClients[recvPacket.cid]->GetCurrentRoom();
        int roomID = pRoom->GetID();
        pRoom->LeaveUser(recvPacket.cid);

        //  남아있는 플레이어에게 나간 플레이어의 정보를, 나간 플레이어에게는 roomListInfo를 보내준다.
        SendRoomOutPlayerAndRoomList(pRoom, pClients[recvPacket.cid]);

        break;
    }
    case CS_PACKET_TYPE::ready: {
        CS_READY& recvPacket = GetPacket<CS_READY>();
        cout << format("CS_READY : cid - {}, pid - {} \n", recvPacket.cid, recvPacket.pid);

        Client* pClient = pClients[recvPacket.cid];
        Room* pRoom = pClient->GetCurrentRoom();
        // 1. ready패킷을 보낸 플레이어가 방장일 경우
        if (pRoom->GetHostID() == pClient->GetClientID()) {
            // 1-2. 다른 플레이어들이 모두 레디인지 확인하고 모두 레디한 상태라면 게임을 시작한다.
            
            if (pRoom->GetNumOfParticipants() >= 1) {
                bool success = true;
                cout << pRoom->GetParticipants().size() << " : ";
                for (UINT participant : pRoom->GetParticipants()) {
                    if (participant == pRoom->GetHostID())  // 방장은 제외하고
                        continue;
                    if (pClients[participant]->GetClientState() != ClientState::roomReady) {
                        success = false;
                        cout << "레디를 하지 않은 클라 ID : " << participant << "\n";
                        break;
                    }
                }
                if (success) {  // 게임시작
                    pRoom->GameStart();
                }
                else {  // 시작불가 - 레디하지않은 인원이 존재한다.
                    SC_FAIL sendPacket;
                    sendPacket.cause = SC_FAIL_TYPE::notAllReady;
                    SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
                }
            }
            else {  // 시작불가 - 인원부족
                SC_FAIL sendPacket;
                sendPacket.cause = SC_FAIL_TYPE::lackOfParticipants;
                SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            }
        }
        // 2. ready패킷을 보낸 플레이어가 방장이 아닐 경우
        else {
            cout << (int)pClient->GetClientState() << "\n";
            if (pClient->GetClientState() == ClientState::roomWait)
                pClient->SetClientState(ClientState::roomReady);
            else if (pClient->GetClientState() == ClientState::roomReady)
                pClient->SetClientState(ClientState::roomWait);

            SC_READY sendPacket;
            sendPacket.readyClientID = pClient->GetClientID();
            //해당플레이어가 레디했다는 것을 알린다.
            for (UINT participant : pRoom->GetParticipants())
                SendContents(pClients[participant]->GetSocket(), pClients[participant]->GetRemainBuffer(), sendPacket);
        }
        break;
    }
    case CS_PACKET_TYPE::loadingComplete: {
        CS_LOADING_COMPLETE& recvPacket = GetPacket<CS_LOADING_COMPLETE>();
        cout << format("CS_LOADING_COMPLETE : cid - {}, roomID - {}, pid - {} \n", recvPacket.cid, recvPacket.roomID, recvPacket.pid);

        cout << recvPacket.roomID << " 번 방 로딩 완료 ! \n";
        PlayInfo* pPlayInfo = pPlayInfos[recvPacket.roomID];
        pPlayInfo->LoadingComplete(recvPacket.cid);
        break;
    }

    case CS_PACKET_TYPE::playerInfo:
    case CS_PACKET_TYPE::toggleDoor:
    case CS_PACKET_TYPE::useWaterDispenser:
    case CS_PACKET_TYPE::queryUseComputer:
    case CS_PACKET_TYPE::hackingRate: 
    case CS_PACKET_TYPE::attack: 
    case CS_PACKET_TYPE::hit: 
    case CS_PACKET_TYPE::goPrison: 
    case CS_PACKET_TYPE::openPrisonDoor: 
    case CS_PACKET_TYPE::useItem:
    case CS_PACKET_TYPE::removeItem:
    case CS_PACKET_TYPE::toggleLever:
    case CS_PACKET_TYPE::removeTrap:
    case CS_PACKET_TYPE::exitPlayer:
    case CS_PACKET_TYPE::exitGame:
    {
        READ_CID_IN_PACKET& readFrontPart = GetPacket<READ_CID_IN_PACKET>();
        //cout << format("READ_CID_IN_PACKET : {}, cid - {}\n", (int)readFrontPart.packetType, readFrontPart.cid);

        pClients[readFrontPart.cid]->GetCurrentPlayInfo()->ProcessRecv(packetType);
        break;
    }

    default:
        READ_CID_IN_PACKET readFrontPart = GetPacket<READ_CID_IN_PACKET>();
        cout << format("잘못된 패킷 번호 : {}, cid - {}\n", (int)readFrontPart.packetType, readFrontPart.cid);
        break;
    }
}

void ServerFramework::FrameAdvance() {
	// 게임을 진행시키면서 필요에 따라 메시지를 전송한다.
    
    float timeElapsed = chrono::duration_cast<chrono::milliseconds>((chrono::system_clock::now() - lastTime)).count() / 1'000.f;

    float period = SERVER_PERIOD;

    if (period <= timeElapsed)
        lastTime = chrono::system_clock::now();
    else
        return;
    
    for (auto [playInfoID, pPlayInfo] : pPlayInfos)
        pPlayInfo->FrameAdvance(timeElapsed);

    //// FPS 표시
    //wstring titleString = L"FPS : " + to_wstring(timer.GetFPS());
    //SetWindowText(windowHandle, (LPCWSTR)titleString.c_str());
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
        
        
        Client* pClient = pClients[_clientID];

        // 클라이언트가 게임중이었을 경우 그 게임에서 탈주시킨다.
        PlayInfo* pPlayInfo = pClient->GetCurrentPlayInfo();
        if (pPlayInfo)
            pPlayInfo->EscapeClient(_clientID);

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

        cout << format("<- 클라이언트 종료 : clientID - {0}\n", _clientID);
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

void ServerFramework::SendRoomlistInfo(Client* _pClient, int _page) {
    // 보낼 데이터를 만든다.

    SC_ROOMLIST_INFO sendPacket;

    // 2. 각 방에 대한 정보를 삽입
    for (int index = 0; auto [roomID, pRoom] : pRooms | views::drop(_page * 6 - 6) | views::take(6)) {
        SC_SUB_ROOMLIST_INFO& roomInfo = sendPacket.roomInfo[index];
        roomInfo.nParticipant = pRoom->GetNumOfParticipants();
        roomInfo.roomID = roomID;
        roomInfo.started = pRoom->IsGameRunning();
        ++index;
        cout << "roomID : " << roomID << "\n";
        cout << "roomInfo.nParticipant : " << roomInfo.nParticipant << "\n";
    }
    SendContents(_pClient->GetSocket(), _pClient->GetRemainBuffer(), sendPacket);
}

void ServerFramework::AddPlayInfo(UINT _roomID) {

    PlayInfo* pPlayInfo = new PlayInfo(_roomID);
    pPlayInfos[_roomID] = pPlayInfo;
    pPlayInfo->Init(_roomID);

}

void ServerFramework::RemovePlayInfo(UINT _playInfoID) {
    if (pPlayInfos.contains(_playInfoID)) {    // 참가자가 한명도 없을 경우
        cout << _playInfoID << "번 게임을 삭제합니다.\n";

        delete pPlayInfos[_playInfoID];
        pPlayInfos.erase(_playInfoID);
    }
}

void ServerFramework::SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient) {
    // 2. 방에 존재하는 플레이에게 나간 플레이어의 정보를 전송한다.
    SC_ROOM_OUT_PLAYER_INFO sendPacket;
    sendPacket.outClientID = pOutClient->GetClientID();
    sendPacket.newHostID = pRoom->GetHostID();
    for (UINT clientID : pRoom->GetParticipants()) {
        cout << clientID << "플레이어에게 " << sendPacket.outClientID << "플레이어가 나갔다고 패킷 전송함\n";
        SendContents(pClients[clientID]->GetSocket(), pClients[clientID]->GetRemainBuffer(), sendPacket);
    }

    // 3. 나간 플레이어에게 방의 리스트 정보를 보낸다.
    SendRoomlistInfo(pOutClient, 1);
}

void ServerFramework::LoadMapFile() {

    ifstream mapFile("Map", ios::binary);

    if (!mapFile) {
        cout << "Map File Load Failed!\n";
        return;
    }

    // 맵내 전체 인스턴스의 개수
    UINT nInstance;
    string objName;
    SectorLayer objLayer;
    ObjectType objType;
    XMFLOAT3 position, scale;
    XMFLOAT4 rotation;
    GameObject* pObject = nullptr;

    // nInstance (UINT)
    mapFile.read((char*)&nInstance, sizeof(UINT));

    for (UINT i = 0; i < nInstance; ++i) {
        // nameSize(UINT) / fileName (string)
        ReadStringBinary(objName, mapFile);

        // objectLayer(char)
        mapFile.read((char*)&objLayer, sizeof(SectorLayer));
        // objectType(char)
        mapFile.read((char*)&objType, sizeof(ObjectType));

        // position(float * 3) / scale(float * 3) / rotation(float * 3)
        mapFile.read((char*)&position, sizeof(XMFLOAT3));
        mapFile.read((char*)&scale, sizeof(XMFLOAT3));
        mapFile.read((char*)&rotation, sizeof(XMFLOAT4));
        
        switch (objType) {

            // break;가 없는 것에 주의
        case ObjectType::Ldoor:
        case ObjectType::Rdoor:
        case ObjectType::exitLDoor:
        case ObjectType::exitRDoor:
        case ObjectType::prisonDoor:
            if (objType == ObjectType::Ldoor || objType == ObjectType::Rdoor || objType == ObjectType::prisonDoor)
                pObject = new Door();
            else if (objType == ObjectType::exitLDoor || objType == ObjectType::exitRDoor) {
                pObject = new Door();
                static_cast<Door*>(pObject)->SetExitDoor(true);
            }
            __fallthrough;
        case ObjectType::lever:
            if (objType == ObjectType::lever)
                pObject = new Lever();
            __fallthrough;
        case ObjectType::waterDispenser: 
            if (objType == ObjectType::waterDispenser)
                pObject = new WaterDispenser();
            __fallthrough;
        case ObjectType::computer:
            if (objType == ObjectType::computer)
                pObject = new Computer();

            pObject->SetType(objType);
            pObject->SetID(objectIDStart);
            pInitialObjects.emplace_back(pObject->GetID(), pObject);
            break;

        case ObjectType::studentStartPosition:
            studentStartPositions.push_back(position);
            break;
        case ObjectType::professorStartPosition:
            professorStartPosition = position;
            break;
        case ObjectType::prisonPosition:
            prisonPosition = position;
            break;
        case ObjectType::prisonExitPosition:
            prisonExitPosition = position;
            break;
        case ObjectType::itemSpawnLocation:
            itemSpawnLocationCount++;
            break;
        case ObjectType::exitBox: {
            XMFLOAT3 buffer; 
            mapFile.read((char*)&buffer, sizeof(XMFLOAT3));
            mapFile.read((char*)&buffer, sizeof(XMFLOAT3));
        }
            break;
        default:
            break;
        }

        ++objectIDStart;
	}

}

const vector<XMFLOAT3>& ServerFramework::GetShuffledStudentStartPositions() {
    ranges::shuffle(studentStartPositions, rd);
    return studentStartPositions;
}
