// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")    // 콘솔창 띄우기( 테스트를 위한 용도 )
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

//#define DEBUG

// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <mmsystem.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

// io
#include <iostream>

// 스마트 포인터 
#include <memory>

// 컨테이너
#include <vector>
#include <array>
#include <map>
#include <ranges>
#include <queue>
#include <stack>
#include <format>
#include <fstream>

#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

// file로 부터 string을 읽는다.
void ReadStringBinary(string& _dest, ifstream& _file);
void ReadXMFLOAT3Binary(XMFLOAT3& _dest);

namespace Vector3 {
	inline XMFLOAT3 Normalize(const XMFLOAT3& _vector) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&_vector)));
		return(result);
	}
	inline XMFLOAT3 Normalize(float _x, float _y, float _z) {
		XMFLOAT3 result(_x, _y, _z);
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&result)));
		return(result);
	}

	inline XMFLOAT3 ScalarProduct(const XMFLOAT3& _vector, float _scalar) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector) * _scalar);
		return result;
	}
	inline XMFLOAT3 Add(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) + XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Add(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2, float _scalar) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) + (XMLoadFloat3(&_vector2) * _scalar));
		return result;
	}

}

namespace Matrix4x4 {
	inline XMFLOAT4X4 Identity() {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}

	inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& _matrix1, const XMFLOAT4X4& _matrix2) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMLoadFloat4x4(&_matrix1) * XMLoadFloat4x4(&_matrix2));
		return(result);
	}
}

