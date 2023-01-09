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
	clientIDCount = 0;
    roomIDCount = 0;
}
ServerFramework::~ServerFramework() {
}

///  �Ϲ��Լ�
void ServerFramework::ProcessSocketMessage(HWND _hWnd, UINT _message, WPARAM _wParam, LPARAM _lParam) { 
    /// wParam : ����, lParam : select�� �ʵ�
    
    // ������ ��ſ� ����� ����
    int result;

    // ���� �߻� ���� Ȯ��
    if (WSAGETSELECTERROR(_lParam)) {
        SockErrorDisplay(WSAGETSELECTERROR(_lParam));
        RemoveClient(_wParam);   // �ش� ����(wParam)�� Ŭ���̾�Ʈ ����
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
        AddClient(new Client(clientSocket));

        // ����Ʈ �𵨿� (�߰��� Ŭ���̾�Ʈ��)������ ���
        result = WSAAsyncSelect(clientSocket, _hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
        if (result == SOCKET_ERROR) {
            SockErrorDisplay("WSAAsyncSelect()");
            RemoveClient(clientSocket);
        }
        break;
    }
    case FD_READ:
    case FD_WRITE:
        ProcessRecv(_wParam);
        break;
    case FD_CLOSE:
        RemoveClient(_wParam);
        break;
    }
}
void ServerFramework::ProcessRecv(SOCKET _socket) {
    char packetType;
    recv(_socket, &packetType, sizeof(char), 0);

    switch (packetType) {
    case 0: {
        TEST_PACKET testPacket;
        // packetType�� ���� �о���ϴ� ũ�⸦ �޾ƿ´�.
        recv(_socket, (char*)&testPacket + sizeof(char), sizeof(int) + sizeof(char), 0);
        cout << format("���� : {}\n", testPacket.letter);

        testPacket.letter = 'p';
        send(pClients[testPacket.cid]->GetSocket(), (char*)&testPacket, sizeof(TEST_PACKET), 0);

    }
        break;
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
        cout << format("-> Ŭ���̾�Ʈ ���� : clientID - {0}\n", _pClient->GetClientID());
        ++clientIDCount;    // clientIDCount�� ������Ų��.
	}
}
void ServerFramework::RemoveClient(int _clientID) {
    if (pClients.contains(_clientID)) {
        cout << format("<- Ŭ���̾�Ʈ ���� : clientID - {0}\n", pClients[_clientID]->GetClientID());
        delete pClients[_clientID];
        pClients.erase(_clientID);
    }
}
