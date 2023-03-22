
#include "stdafx.h"
#include "Client.h"
#include "GameFramework.h"

#define MAX_LOADSTRING 100
HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            
HWND hLoginDlg;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
void ConnectToServer();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    setlocale(LC_ALL, "");
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;
    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {

            GameFramework::Instance().FrameAdvance();
        }
    }
    //GameFramework::Destroy();


    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_CLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU |
        WS_BORDER;

    RECT windowSize = { 0, 0, C_WIDTH, C_HEIGHT };
    AdjustWindowRect(&windowSize, dwStyle, FALSE);

    hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT,
        CW_USEDEFAULT, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, NULL, NULL, hInstance,
        NULL);

    if (!hWnd)
    {
        return FALSE;
    }


    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    GameFramework::Create(hInst, hWnd);

    GetClientRect(hWnd, &clientRect);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT p;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
        GetCursorPos(&p);   // 현재 커서위치를 구함
        ScreenToClient(hWnd, &p);   // 현재 윈도우의 클라이언트 영역으로 좌표를 바꿈
        GameFramework::Instance().ProcessMouseInput(message, GetViewportCoord(p));  // 뷰포트 좌표계로 변환
        break;
    case WM_SOCKET:

        if(&GameFramework::Instance()) GameFramework::Instance().ProcessSocketMessage(hWnd, message, wParam, lParam);
        else {
            // 프레임워크가 생성되지 않은 상태에서 오는 메시지를 처리
            ProcessSocketMessage(hWnd, message, wParam, lParam);
        }
        break;
        // 클라이언트가 종료할 때 서버에게 종료사실을 알려준다.
    case WM_CLOSE:
    case WM_DESTROY:
        GameFramework::Instance().NoticeCloseToServer();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        hLoginDlg = hDlg;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // 입력된 닉네임과 아이피를 가져온다.
            WCHAR nickNameBuffer[64];
            TCHAR IPbuffer[16];
            GetDlgItemTextW(hDlg, IDC_EDIT1, nickNameBuffer, 64);
            GetDlgItemText(hDlg, IDC_IPADDRESS1, IPbuffer, 16);
            size_t nicklen = wcslen(nickNameBuffer);

            if (nicklen > 20)
                MessageBox(hDlg, L"닉네임이 20자 이상입니다.", L"알림", MB_OK);
            else
            {
                if (nicklen != 0) nickName.assign(nickNameBuffer, nickNameBuffer + nicklen + 1);
#ifdef UNICODE
                // 유니코드
                serverIP.assign((WCHAR*)IPbuffer, (WCHAR*)IPbuffer + wcslen((WCHAR*)IPbuffer));
#else
                // ANSI
                serverIP.assign((CHAR*)IPbuffer, (CHAR*)IPbuffer + strlen((CHAR*)IPbuffer));
#endif
                // 먼저 서버와 접속을 시도한다.
                ConnectToServer();

                // 접속 성공후 cid를 받은 후에 중복확인용 패킷을 보낸다.

                // recv 메시지를 받는 곳에서 중복이 아닐경우 해당 dialog를 종료한다.

            }
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void ConnectToServer() {


    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        cout << "WSA Init Error! \n";

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) SockErrorQuit("socket()");

    if (serverIP == "0.0.0.0") {
        serverIP = "127.0.0.1";
    }

    int result = 0;

    cout << serverIP << " 에 접속합니다\n";
    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);


    int retval = connect(server_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    if (retval == SOCKET_ERROR) SockErrorQuit("connect()");

    // WSAAsyncSelect()
    result = WSAAsyncSelect(server_sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
    if (result == SOCKET_ERROR) SockErrorQuit("WSAAsyncSelect()");

}

void ProcessSocketMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) {
    if (WSAGETSELECTERROR(_lParam)) {
        SockErrorDisplay(WSAGETSELECTERROR(_lParam));
        return;
    }
    switch (WSAGETSELECTEVENT(_lParam)) {
    case FD_READ: {
        // 고정길이의 패킷을 Recv받는다.
        static int recvByte = 0;

        int result = RecvFixedPacket(recvByte);
        if (result != SOCKET_ERROR) {
            recvByte += result;
            // 덜 받은 경우
            if (recvByte < BUFSIZE) {
                return;
            }
            // 모두 받았을 경우
            else {
                recvByte = 0;
            }
        }
        else {
            // EWOULDBLOCK
            return;
        }
        SC_PACKET_TYPE packetType = (SC_PACKET_TYPE)recvBuffer[0];

        switch (packetType) {
        case SC_PACKET_TYPE::giveClientID: { // 처음 접속시 플레이어 cid를 부여받는 패킷
            SC_GIVE_CLIENT_ID* recvPacket = GetPacket<SC_GIVE_CLIENT_ID>();
            cid = recvPacket->clientID;
            cout << "내 클라이언트 id = " << cid << "\n";
            if (nickName.size() == 0) {
                nickName = L"Player" + to_wstring(cid);
            }
            CS_CHECK_EXIST_NICKNAME packet;
            packet.cid = cid;
            for (int i = 0; i < nickName.size(); ++i) {
                packet.nickname[i] = nickName[i];
            }
            SendFixedPacket(packet);
            break;
        }
        case SC_PACKET_TYPE::checkNickname: {
            SC_CHECK_NICKNAME* packet = GetPacket<SC_CHECK_NICKNAME>();
            // 닉네임이 중복일경우 다시 입력받는다.
            cout << "nickName : ";
            wcout << nickName;
            if (packet->isExist)
                MessageBox(hLoginDlg, L"닉네임이 중복됩니다.", L"알림", MB_OK);
            else {
                // 닉네임이 중복이 아닐경우 DialogBox를 없애고, 게임을 시작한다.
                EndDialog(hLoginDlg, NULL);
            }
            break;
        }
        default:
            break;
        }
        break;

    }
    case FD_CLOSE:
        cout << "서버가 종료되었습니다.\n";
        break;
}
}
