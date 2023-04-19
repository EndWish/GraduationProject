
#pragma once


#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")    // 콘솔창 띄우기 ( 테스트를 위한 용도 )

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외
#define NUM_G_BUFFER 6
#define NUM_SHADOW_MAP 6
#define MAX_LIGHTS 20 // 조명의 최대 갯수
#define GRAVITY 9.8f
#define C_WIDTH 1280	
#define C_HEIGHT 720
#define USING_INSTANCING
//#define DEBUG
#define DRAW_BOUNDING



// Windows 헤더 파일
#include <windows.h>
#include <random>

// C 런타임 헤더 파일
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <exception>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <mmsystem.h>

// directX 헤더 파일
#include <d3d11.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <dsound.h>

#include <mmsystem.h>

// 서버와 약속한 프로토콜
#include "../protocol.h"

// com_error 디버그용
#include <comdef.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dsound.lib")

#include <SDKDDKVer.h>

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
#include <unordered_map>

#include <numeric>
#include <numbers>
#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#define MAX_BONE 100

// 각 디스크립터 내 보더 사이즈
extern UINT cbvSrvDescriptorIncrementSize;
extern UINT	rtvDescriptorIncrementSize;
extern UINT dsvDescriptorIncrementSize;

// 메인 윈도우 핸들
extern HWND hWnd;

// 서버에 대한 소켓
extern SOCKET server_sock;

// 현재 클라이언트가 서버에게 부여받은 고유 클라이언트 id
extern UINT cid;
extern UINT myObjectID;
extern bool isPlayerProfessor;
extern bool AllLeverPowerOn;
extern UINT packetIDCount;
extern XMFLOAT3 prisonPosition, prisonExitPosition;

// 현재 클라이언트 크기
extern RECT clientRect;
extern random_device rd;

// 플레이어의 닉네임
extern WCHAR nickname[20];

// 서버의 IP
extern string serverIP;

extern UINT packetIDCount;
// 고정 길이 패킷을 담을 버퍼

extern array<char, BUFSIZE> sendBuffer;
extern array<char, BUFSIZE> recvBuffer;

using Microsoft::WRL::ComPtr;
extern int testcount;


// float 난수 생성
float random(float min, float max);

// file로 부터 string을 읽는다.
void ReadStringBinary(string& _dest, ifstream& _file);
void SynchronizeResourceTransition(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const ComPtr<ID3D12Resource>& _pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
// 리소스 생성
ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates, const ComPtr<ID3D12Resource>& _pUploadBuffer);

//xmfloat, xmint 출력하기
std::ostream& operator<<(std::ostream& os, const XMFLOAT2& f2);
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4X4& f4x4);
std::ostream& operator<<(std::ostream& os, const XMINT3& i3);


ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates);
ComPtr<ID3D12Resource> CreateTexture2DResource(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

// 소켓 함수 오류 출력
void SockErrorQuit(const char* msg);
void SockErrorDisplay(const char* msg);
void SockErrorDisplay(int errcode);

XMFLOAT2 GetWorldToScreenCoord(const XMFLOAT3& _position, const XMFLOAT4X4& _view, const XMFLOAT4X4& _proj);
// 마우스 클릭시 해당 좌표를 뷰포트 좌표계로 변경
XMFLOAT2 GetViewportCoord(POINT _point);

int RecvFixedPacket(int _recvByte);

// 패킷의 크기만큼에서 패킷의 타입 크기만큼을 제외한 실제 패킷의 내용만 Recv하는 함수
template <class Packet>
int SendFixedPacket(Packet& _packet) {
	_packet.pid = packetIDCount++;
	memcpy(sendBuffer.data(), (char*)&_packet, sizeof(_packet));
	int result = send(server_sock, sendBuffer.data(), BUFSIZE, 0);
	if (result == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			// wouldblock이 아닐 경우 오류를 출력한다.
			SockErrorDisplay("Send()");
		}
		else {
			cout << "send wouldblock!!\n";
			return result;
		}
	}
	return result;
}

template <class Packet>
Packet* GetPacket() {
	return reinterpret_cast<Packet*>(recvBuffer.data());
}

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
	inline XMFLOAT3 Subtract(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) - XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Transform(const XMFLOAT3& _vector, const XMFLOAT4X4& _matrix) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Transform(XMLoadFloat3(&_vector), XMLoadFloat4x4(&_matrix)));
		return result;
	}
	inline float DotProduct(XMFLOAT3& _vector1, XMFLOAT3& _vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result.x);
	}
	// 벡터 나누기
	inline XMFLOAT3 Division(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) / XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Division(const XMFLOAT3& _vector1, const  XMINT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) / XMLoadSInt3(&_vector2));
		return result;
	}
	// 벡터 원소 곱
	inline XMFLOAT3 Multiple(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) * XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Multiple(const XMFLOAT3& _vector1, const  XMINT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) * XMLoadSInt3(&_vector2));
		return result;
	}
	// 집게
	inline XMFLOAT3 Clamp(const XMFLOAT3& _vector, const  XMFLOAT3& _min, const  XMFLOAT3& _max) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVectorClamp(XMLoadFloat3(&_vector), XMLoadFloat3(&_min), XMLoadFloat3(&_max)));
		return result;
	}
	inline XMINT3 Clamp(const XMINT3& _vector, const  XMINT3& _min, const  XMINT3& _max) {
		XMINT3 result;
		XMStoreSInt3(&result, XMVectorClamp(XMLoadSInt3(&_vector), XMLoadSInt3(&_min), XMLoadSInt3(&_max)));
		return result;
	}
	inline XMFLOAT3 CrossProduct(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result);
	}

	inline float Angle(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if(bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3AngleBetweenNormals(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		else 
			XMStoreFloat3(&xmf3Result, XMVector3AngleBetweenVectors(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(XMConvertToDegrees(xmf3Result.x));
	}

	inline bool IsSame(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2) {
		return XMVector3Equal(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2));
	}

	inline XMFLOAT3 Lerp(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2, float _t) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVectorLerp(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2), _t));
		return xmf3Result;
	}

	inline float Length(const XMFLOAT3& _vector) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&_vector)));
		return xmf3Result.x;
	}

	inline float Length(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&_vector1) - XMLoadFloat3(&_vector2)));
		return xmf3Result.x;
	}

	inline float LengthEst(const XMFLOAT3& _vector) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3LengthEst(XMLoadFloat3(&_vector)));
		return xmf3Result.x;
	}

	inline float LengthEst(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3LengthEst(XMLoadFloat3(&_vector1) - XMLoadFloat3(&_vector2)));
		return xmf3Result.x;
	}

	inline float LengthSq(const XMFLOAT3& _vector) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3LengthSq(XMLoadFloat3(&_vector)));
		return xmf3Result.x;
	}
	inline XMFLOAT3 Rotate(const XMFLOAT3& _vector, const XMFLOAT4& _rotation) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Rotate(XMLoadFloat3(&_vector), XMLoadFloat4(&_rotation)));
		return xmf3Result;
	}

}

namespace Vector4 {
	inline XMFLOAT4 Add(const XMFLOAT4& _vector1, const XMFLOAT4& _vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&_vector1) + XMLoadFloat4(&_vector2));
		return(xmf4Result);
	}
	
	inline XMFLOAT4 QuaternionMultiply(const XMFLOAT4& _quaternion1, const XMFLOAT4& _quaternion2) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionMultiply(XMLoadFloat4(&_quaternion1), XMLoadFloat4(&_quaternion2)));
		return result;
	}

	inline XMFLOAT4 QuaternionRotation(const XMFLOAT3& _axis, float _angle) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionRotationAxis(XMLoadFloat3(&_axis), XMConvertToRadians(_angle)));
		return result;
	}

	inline XMFLOAT4 Transform(const XMFLOAT4& _vector, const XMFLOAT4X4& _matrix) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVector4Transform(XMLoadFloat4(&_vector), XMLoadFloat4x4(&_matrix)));
		return result;
	}

	inline XMFLOAT4 Multiply(const XMFLOAT4& _vector1, const XMFLOAT4& _vector2)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&_vector1) * XMLoadFloat4(&_vector2));
		return result;
	}

	inline XMFLOAT4 Multiply(float _scalar,const XMFLOAT4& _vector)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, _scalar * XMLoadFloat4(&_vector));
		return result;
	}
	inline XMFLOAT4 QuaternionIdentity() {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionIdentity());
		return result;
	}
	inline XMFLOAT4 QuaternionSlerp(const XMFLOAT4& _quaternion1, const XMFLOAT4& _quaternion2, float t) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionSlerp(XMLoadFloat4(&_quaternion1), XMLoadFloat4(&_quaternion2), t));
		return result;
	}
	inline bool IsSame(const XMFLOAT4& _vector1, const XMFLOAT4& _vector2) {
		return XMVector4Equal(XMLoadFloat4(&_vector1), XMLoadFloat4(&_vector2));
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

	inline XMFLOAT4X4 RotationAxis(const XMFLOAT3& _axis, float _angle) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationAxis(XMLoadFloat3(&_axis), XMConvertToRadians(_angle));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 RotateQuaternion(const XMFLOAT4& _quaternion) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&_quaternion));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 RotatePitchYawRoll(float _pitch, float _yaw, float _roll) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(_pitch), XMConvertToRadians(_yaw), XMConvertToRadians(_roll));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 LookAtLH(const XMFLOAT3& _eyePosition, const XMFLOAT3& _lookAtPosition, const XMFLOAT3& _upDirection) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixLookAtLH(XMLoadFloat3(&_eyePosition), XMLoadFloat3(&_lookAtPosition), XMLoadFloat3(&_upDirection)));
		return(result);
	}
	inline XMFLOAT4X4 PerspectiveFovLH(float _fovAngleY, float _aspectRatio, float _nearZ, float _farZ) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(_fovAngleY, _aspectRatio, _nearZ, _farZ));
		return result;
	}
	inline XMFLOAT4X4 ScaleTransform(const XMFLOAT3& scale) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixScaling(scale.x, scale.y, scale.z));
		return result;
	}
	inline XMFLOAT4X4 MoveTransform(const XMFLOAT3& _move) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixTranslationFromVector(XMLoadFloat3(&_move)));
		return result;
	}
}
