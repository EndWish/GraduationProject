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
	clientIDCount = 0;
}
ServerFramework::~ServerFramework() {
}

///  일반함수
void ServerFramework::ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { 
    /// wParam : 소켓, lParam : select의 필드
    
    // 데이터 통신에 사용할 변수
    int result;

    // 오류 발생 여부 확인
    if (WSAGETSELECTERROR(lParam)) {
        SockErrorDisplay(WSAGETSELECTERROR(lParam));
        RemoveClient(wParam);   // 해당 소켓(wParam)의 클라이언트 제거
        return;
    }

    // 메시지 처리
    switch (WSAGETSELECTEVENT(lParam)) {
        // 새로운 접속이 왔을 때:
    case FD_ACCEPT: {

        // 새로운 클라이언트의 접속을 받는다.
        struct sockaddr_in clientaddr;
        int addrSize = sizeof(clientaddr);
        SOCKET clientSocket = accept(wParam, (struct sockaddr*)&clientaddr, &addrSize);
        if (clientSocket == INVALID_SOCKET) {
            SockErrorDisplay("accept()");
            return;
        }
        cout << format("클라이언트 접속 : IP 주소={0}, 포트 번호={1}\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 새로운 클라이언트를 추가한다. (컨테이너에 추가)
        Client* newClient = new Client(clientIDCount++, clientSocket);
        AddClient(newClient);

        // 셀렉트 모델에 (추가된 클라이언트의)소켓을 등록
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
	// 게임을 진행시키면서 필요에 따라 메시지를 전송한다.
}
void ServerFramework::AddClient(Client* _pClient) {
	if (_pClient) {
		pClients.emplace(_pClient->GetSocket(), _pClient);
        cout << format("-> 클라이언트 접속 : clientID - {0}\n", _pClient->GetClientID());
	}
}
void ServerFramework::RemoveClient(const SOCKET& _socket) {
    if (_socket) {
        cout << format("<- 클라이언트 종료 : clientID - {0}\n", pClients[_socket]->GetClientID());
        delete pClients[_socket];
        pClients.erase(_socket);
        
    }
}
