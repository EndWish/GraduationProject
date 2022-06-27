// stdafx.h : 자주 사용하지만 자주 변경되지는 않는 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 들어 있는 포함 파일입니다.

#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>	// Windows 헤더 파일

#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")	// 콘솔창 띄우기( 테스트를 위한 용도 )
#include <iostream>	//테스트용

// C의 런타임 헤더 파일
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>
#include <shellapi.h>

#include <fstream>
#include <cmath>
#include <string>
#include <array>
#include <queue>
#include <stack>

#define DEBUG

using namespace std;

// 다이렉트X 관련 헤더파일
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <Mmsystem.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

using namespace std;

using Microsoft::WRL::ComPtr;	// #include <wrl.h>, COM 객체를 위한 스마트 포인터이다

#define FRAME_BUFFER_WIDTH		1920
#define FRAME_BUFFER_HEIGHT		1080

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")