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
}
ServerFramework::~ServerFramework() {
}

///  �Ϲ��Լ�
void ServerFramework::ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { 
    /// wParam : ����, lParam : select�� �ʵ�
    
    // ������ ��ſ� ����� ����
    int result;

    // ���� �߻� ���� Ȯ��
    if (WSAGETSELECTERROR(lParam)) {
        SockErrorDisplay(WSAGETSELECTERROR(lParam));
        RemoveClient(wParam);   // �ش� ����(wParam)�� Ŭ���̾�Ʈ ����
        return;
    }

    // �޽��� ó��
    switch (WSAGETSELECTEVENT(lParam)) {
        // ���ο� ������ ���� ��:
    case FD_ACCEPT: {

        // ���ο� Ŭ���̾�Ʈ�� ������ �޴´�.
        struct sockaddr_in clientaddr;
        int addrSize = sizeof(clientaddr);
        SOCKET clientSocket = accept(wParam, (struct sockaddr*)&clientaddr, &addrSize);
        if (clientSocket == INVALID_SOCKET) {
            SockErrorDisplay("accept()");
            return;
        }
        cout << format("Ŭ���̾�Ʈ ���� : IP �ּ�={0}, ��Ʈ ��ȣ={1}\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ���ο� Ŭ���̾�Ʈ�� �߰��Ѵ�. (�����̳ʿ� �߰�)
        Client* newClient = new Client(clientIDCount++, clientSocket);
        AddClient(newClient);

        // ����Ʈ �𵨿� (�߰��� Ŭ���̾�Ʈ��)������ ���
        result = WSAAsyncSelect(clientSocket, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
        if (result == SOCKET_ERROR) {
            SockErrorDisplay("WSAAsyncSelect()");
            RemoveClient(clientSocket);
        }
        break;
    }
    case FD_READ:
    case FD_WRITE:
        break;
    case FD_CLOSE:
        RemoveClient(wParam);
        break;
    }
}
void ServerFramework::FrameAdvance() {
	// ������ �����Ű�鼭 �ʿ信 ���� �޽����� �����Ѵ�.
}
void ServerFramework::AddClient(Client* _pClient) {
	if (_pClient) {
		pClients.emplace(_pClient->GetSocket(), _pClient);
        cout << format("-> Ŭ���̾�Ʈ ���� : clientID - {0}\n", _pClient->GetClientID());
	}
}
void ServerFramework::RemoveClient(const SOCKET& _socket) {
    if (_socket) {
        cout << format("<- Ŭ���̾�Ʈ ���� : clientID - {0}\n", pClients[_socket]->GetClientID());
        delete pClients[_socket];
        pClients.erase(_socket);
        
    }
}
