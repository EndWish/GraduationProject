/// 헤더파일 추가
#pragma once

#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")    // 콘솔창 띄우기( 테스트를 위한 용도 )

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "../protocol.h"

//// 서버 관련 헤더파일
//#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
//#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기
//#include <winsock2.h> // 윈속2 메인 헤더
//#include <ws2tcpip.h> // 윈속2 확장 헤더
//#pragma comment(lib, "ws2_32") // ws2_32.lib 링크
//
//// 추가로 필요한 헤더파일
//#define SERVERPORT 9000
//#define WM_SOCKET  (WM_USER+1)
#include <stdio.h>
#include <iostream>
#include <format> // c++ lastest
#include <vector>
#include <string>
#include <array>
#include <unordered_map>

using namespace std;

/// 전역 함수
// 소켓 함수 오류 출력
void SockErrorQuit(const char* msg);
void SockErrorDisplay(const char* msg);
void SockErrorDisplay(int errcode);