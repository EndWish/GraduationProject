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

#include <memory>
#include <fstream>
#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include <ranges>
#include <algorithm>

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

extern ID3D12Resource* CreateBufferResource(const ComPtr<ID3D12Device>& pDevice,const ComPtr<ID3D12GraphicsCommandList>& pCommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ID3D12Resource** ppUploadBuffer = NULL);

namespace Vector3
{
	inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmVector, const XMMATRIX& xmTransform) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3TransformCoord(XMLoadFloat3(&xmVector), xmTransform));
		return(result);
	}

	inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmVector, const XMFLOAT4X4& matrix) {
		return(TransformCoord(xmVector, XMLoadFloat4x4(&matrix)));
	}
}

namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity() {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}
	inline void Identity(XMFLOAT4X4& dest) {
		XMStoreFloat4x4(&dest, XMMatrixIdentity());
	}

	inline XMFLOAT4X4 LookAtLH(const XMFLOAT3& eyePosition, const XMFLOAT3& lookAtPosition, const XMFLOAT3& upDirection) {
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&eyePosition), XMLoadFloat3(&lookAtPosition), XMLoadFloat3(&upDirection)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 PerspectiveFovLH(float FOVAngleY, float aspectRatio, float nearZ, float farZ) {
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FOVAngleY, aspectRatio, nearZ, farZ));
		return(xmmtx4x4Result);
	}

}
