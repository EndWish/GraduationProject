#include "stdafx.h"
#include "ServerFramework.h"
#include "GameObject.h"

/// ���� ����
ServerFramework ServerFramework::instance;	// ���� �����ӿ�ũ 

/// ���� �Լ�
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

/// ������ �� �Ҹ���
ServerFramework::ServerFramework() {
    windowHandle = HWND();


	clientIDCount = 1;
    roomIDCount = 1;
    professorStartPosition = XMFLOAT3(0, 0, 0);
    itemSpawnLocationCount = 0;
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

        //int optVal = 300 * 50;
        //setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, sizeof(optVal));

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

    // ��ŶŸ���� �д´�.
    CS_PACKET_TYPE& packetType = GetPacket<CS_PACKET_TYPE>();

    memcpy((char*)&packetType, globalRecvBuffer.data(), sizeof(packetType));

    switch (packetType) {
    case CS_PACKET_TYPE::checkNickname: {
        // �г��� �ߺ� üũ�� �ϰ� �ߺ��� ������� �� ���̵� �����Ѵ�.
        SC_CHECK_NICKNAME sendPacket;
        sendPacket.isExist = false;
        CS_CHECK_EXIST_NICKNAME& recvPacket = GetPacket<CS_CHECK_EXIST_NICKNAME>();
        for (auto [cid, pCheckClient] : pClients) {
            if (pClients[recvPacket.cid] == pCheckClient) continue;
            if (wcscmp(pCheckClient->GetNickname().c_str(), recvPacket.nickname) == 0) {
                // �г����� ���� ���
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
            // �ߺ��Ǵ� �̸��� ��� �ش� Ŭ���̾�Ʈ�� �ٷ� �������ش�.
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
        // ������ �ޱ�
        CS_QUERY_ROOMLIST_INFO& recvPacket = GetPacket<CS_QUERY_ROOMLIST_INFO>();
        cout << format("CS_QUERY_ROOMLIST_INFO : cid - {}, roomPage - {}, pid - {}  \n", recvPacket.cid, recvPacket.roomPage, recvPacket.pid);

        // �� ����Ʈ�� ���� �����͸� �����.
        SendRoomlistInfo(pClient, recvPacket.roomPage);
        break;
    }
    case CS_PACKET_TYPE::visitRoom: {
        // ������ �ޱ�
        CS_QUERY_VISIT_ROOM& recvPacket = GetPacket<CS_QUERY_VISIT_ROOM>();
        cout << format("CS_QUERY_VISIT_ROOM : cid - {}, visitRoomID - {}, pid - {}  \n", recvPacket.cid, recvPacket.visitRoomID, recvPacket.pid);

        // ���� �������� ���� ���
        if (!pRooms.contains(recvPacket.visitRoomID)) { 
            cout << recvPacket.visitRoomID << "�� ���� �������� ����\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::noExistRoom;
            SendContents(_socket, pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // �ο��� �� á�� ���
        Room* pRoom = pRooms[recvPacket.visitRoomID];
        if (pRoom->GetNumOfParticipants() == MAX_PARTICIPANT ) {
            cout << recvPacket.visitRoomID << "�� ���� �ο��� ���� á��.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomOvercapacity;
            SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // ������ ���۵Ǿ��� ���
        if (pRoom->IsGameRunning()) {
            cout << recvPacket.visitRoomID << "�� ���� ������ ���۵Ǿ���.\n";
            SC_FAIL sendPacket;
            sendPacket.cause = SC_FAIL_TYPE::roomGameStarted;
            SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            break;
        }

        // ������ ������ ���
        // 1. �÷��̾ �����Ų �� ���� ������ �����Ѵ�.
        pRoom->EnterUser(recvPacket.cid);

        // 2. ������ ������ �ִ� �÷��̾� ���� ������ �����Ѵ�.
        SC_ROOM_VISIT_PLAYER_INFO sendPacket;
        memcpy(sendPacket.name, pClients[recvPacket.cid]->GetNickname().c_str(), 20);
        sendPacket.visitClientID = recvPacket.cid;
        for (UINT clientID : pRoom->GetParticipants()) {
            if (clientID == recvPacket.cid)  // ������ �÷��̾��� ��� �����Ѵ�.
                continue;
            SendContents(pClients[clientID]->GetSocket(), pClients[clientID]->GetRemainBuffer(), sendPacket);
            cout << "������ ������ �ִ� �÷��̾� ���� ������ �����Ѵ�.\n";
        }
        break;
    }
    case CS_PACKET_TYPE::outRoom: {
        // ������ �ޱ�
        CS_OUT_ROOM& recvPacket = GetPacket<CS_OUT_ROOM>();
        cout << format("CS_OUT_ROOM : cid - {}, pid - {} \n", recvPacket.cid, recvPacket.pid);

        // 1. �÷��̾ �濡�� ��������.
        Room* pRoom = pClients[recvPacket.cid]->GetCurrentRoom();
        int roomID = pRoom->GetID();
        pRoom->LeaveUser(recvPacket.cid);

        //  �����ִ� �÷��̾�� ���� �÷��̾��� ������, ���� �÷��̾�Դ� roomListInfo�� �����ش�.
        SendRoomOutPlayerAndRoomList(pRoom, pClients[recvPacket.cid]);

        break;
    }
    case CS_PACKET_TYPE::ready: {
        CS_READY& recvPacket = GetPacket<CS_READY>();
        cout << format("CS_READY : cid - {}, pid - {} \n", recvPacket.cid, recvPacket.pid);

        Client* pClient = pClients[recvPacket.cid];
        Room* pRoom = pClient->GetCurrentRoom();
        // 1. ready��Ŷ�� ���� �÷��̾ ������ ���
        if (pRoom->GetHostID() == pClient->GetClientID()) {
            // 1-2. �ٸ� �÷��̾���� ��� �������� Ȯ���ϰ� ��� ������ ���¶�� ������ �����Ѵ�.
            
            if (pRoom->GetNumOfParticipants() >= 1) {
                bool success = true;
                cout << pRoom->GetParticipants().size() << " : ";
                for (UINT participant : pRoom->GetParticipants()) {
                    if (participant == pRoom->GetHostID())  // ������ �����ϰ�
                        continue;
                    if (pClients[participant]->GetClientState() != ClientState::roomReady) {
                        success = false;
                        cout << "���� ���� ���� Ŭ�� ID : " << participant << "\n";
                        break;
                    }
                }
                if (success) {  // ���ӽ���
                    pRoom->GameStart();
                }
                else {  // ���ۺҰ� - ������������ �ο��� �����Ѵ�.
                    SC_FAIL sendPacket;
                    sendPacket.cause = SC_FAIL_TYPE::notAllReady;
                    SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
                }
            }
            else {  // ���ۺҰ� - �ο�����
                SC_FAIL sendPacket;
                sendPacket.cause = SC_FAIL_TYPE::lackOfParticipants;
                SendContents(pClient->GetSocket(), pClient->GetRemainBuffer(), sendPacket);
            }
        }
        // 2. ready��Ŷ�� ���� �÷��̾ ������ �ƴ� ���
        else {
            cout << (int)pClient->GetClientState() << "\n";
            if (pClient->GetClientState() == ClientState::roomWait)
                pClient->SetClientState(ClientState::roomReady);
            else if (pClient->GetClientState() == ClientState::roomReady)
                pClient->SetClientState(ClientState::roomWait);

            SC_READY sendPacket;
            sendPacket.readyClientID = pClient->GetClientID();
            //�ش��÷��̾ �����ߴٴ� ���� �˸���.
            for (UINT participant : pRoom->GetParticipants())
                SendContents(pClients[participant]->GetSocket(), pClients[participant]->GetRemainBuffer(), sendPacket);
        }
        break;
    }
    case CS_PACKET_TYPE::loadingComplete: {
        CS_LOADING_COMPLETE& recvPacket = GetPacket<CS_LOADING_COMPLETE>();
        cout << format("CS_LOADING_COMPLETE : cid - {}, roomID - {}, pid - {} \n", recvPacket.cid, recvPacket.roomID, recvPacket.pid);

        cout << recvPacket.roomID << " �� �� �ε� �Ϸ� ! \n";
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
        cout << format("�߸��� ��Ŷ ��ȣ : {}, cid - {}\n", (int)readFrontPart.packetType, readFrontPart.cid);
        break;
    }
}

void ServerFramework::FrameAdvance() {
	// ������ �����Ű�鼭 �ʿ信 ���� �޽����� �����Ѵ�.
    
    float timeElapsed = chrono::duration_cast<chrono::milliseconds>((chrono::system_clock::now() - lastTime)).count() / 1'000.f;

    float period = SERVER_PERIOD;

    if (period <= timeElapsed)
        lastTime = chrono::system_clock::now();
    else
        return;
    
    for (auto [playInfoID, pPlayInfo] : pPlayInfos)
        pPlayInfo->FrameAdvance(timeElapsed);

    //// FPS ǥ��
    //wstring titleString = L"FPS : " + to_wstring(timer.GetFPS());
    //SetWindowText(windowHandle, (LPCWSTR)titleString.c_str());
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
        
        
        Client* pClient = pClients[_clientID];

        // Ŭ���̾�Ʈ�� �������̾��� ��� �� ���ӿ��� Ż�ֽ�Ų��.
        PlayInfo* pPlayInfo = pClient->GetCurrentPlayInfo();
        if (pPlayInfo)
            pPlayInfo->EscapeClient(_clientID);

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

        cout << format("<- Ŭ���̾�Ʈ ���� : clientID - {0}\n", _clientID);
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

void ServerFramework::SendRoomlistInfo(Client* _pClient, int _page) {
    // ���� �����͸� �����.

    SC_ROOMLIST_INFO sendPacket;

    // 2. �� �濡 ���� ������ ����
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
    if (pPlayInfos.contains(_playInfoID)) {    // �����ڰ� �Ѹ� ���� ���
        cout << _playInfoID << "�� ������ �����մϴ�.\n";

        delete pPlayInfos[_playInfoID];
        pPlayInfos.erase(_playInfoID);
    }
}

void ServerFramework::SendRoomOutPlayerAndRoomList(Room* pRoom, Client* pOutClient) {
    // 2. �濡 �����ϴ� �÷��̿��� ���� �÷��̾��� ������ �����Ѵ�.
    SC_ROOM_OUT_PLAYER_INFO sendPacket;
    sendPacket.outClientID = pOutClient->GetClientID();
    sendPacket.newHostID = pRoom->GetHostID();
    for (UINT clientID : pRoom->GetParticipants()) {
        cout << clientID << "�÷��̾�� " << sendPacket.outClientID << "�÷��̾ �����ٰ� ��Ŷ ������\n";
        SendContents(pClients[clientID]->GetSocket(), pClients[clientID]->GetRemainBuffer(), sendPacket);
    }

    // 3. ���� �÷��̾�� ���� ����Ʈ ������ ������.
    SendRoomlistInfo(pOutClient, 1);
}

void ServerFramework::LoadMapFile() {

    ifstream mapFile("Map", ios::binary);

    if (!mapFile) {
        cout << "Map File Load Failed!\n";
        return;
    }

    // �ʳ� ��ü �ν��Ͻ��� ����
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

            // break;�� ���� �Ϳ� ����
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
