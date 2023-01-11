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

#include "../protocol.h"	// 서버 관련 헤더파일을 포함하고 있다.

// directX 관련 헤더파일
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

#include <stdio.h>
#include <iostream>
#include <format> // c++ lastest
#include <vector>
#include <string>
#include <array>
#include <queue>
#include <random>
#include <unordered_map>
using namespace std;

// 전역 변수
extern random_device rd;

/// 전역 함수
// 소켓 함수 오류 출력
void SockErrorQuit(const char* msg);
void SockErrorDisplay(const char* msg);
void SockErrorDisplay(int errcode);