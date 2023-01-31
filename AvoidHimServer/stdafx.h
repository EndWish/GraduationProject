﻿/// 헤더파일 추가
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
#include <functional>
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

// 패킷의 크기만큼에서 패킷의 타입 크기만큼을 제외한 실제 패킷의 내용만 Recv하는 함수
template <class Packet>
void RecvContents(SOCKET& _sock, Packet& _packet) {
	recv(_sock, (char*)&_packet + sizeof(CS_PACKET_TYPE), sizeof(Packet) - sizeof(CS_PACKET_TYPE), 0);
}

namespace Vector3 {
	// 더하기
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
	// 빼기
	inline XMFLOAT3 Subtract(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) - XMLoadFloat3(&_vector2));
		return result;
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
	// 정규화
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
	// 스칼라 곱
	inline XMFLOAT3 ScalarProduct(const XMFLOAT3& _vector, float _scalar) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector) * _scalar);
		return result;
	}
	// 행렬 곱
	inline XMFLOAT3 Transform(const XMFLOAT3& _vector, const XMFLOAT4X4& _matrix) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Transform(XMLoadFloat3(&_vector), XMLoadFloat4x4(&_matrix)));
		return result;
	}
	// 내적
	inline float DotProduct(XMFLOAT3& _vector1, XMFLOAT3& _vector2) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result.x);
	}
	// 외적
	inline XMFLOAT3 CrossProduct(XMFLOAT3& _vector1, XMFLOAT3& _vector2, bool bNormalize = true) {
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result);
	}
	// 각도
	inline float Angle(XMFLOAT3& _vector1, XMFLOAT3& _vector2) {
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3AngleBetweenVectors(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result.x);
	}
}

namespace Vector4 {
	inline XMFLOAT4 Add(XMFLOAT4& _vector1, XMFLOAT4& _vector2) {
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

	inline XMFLOAT4 Multiply(XMFLOAT4& _vector1, XMFLOAT4& _vector2) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&_vector1) * XMLoadFloat4(&_vector2));
		return result;
	}

	inline XMFLOAT4 Multiply(float _scalar, XMFLOAT4& _vector) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, _scalar * XMLoadFloat4(&_vector));
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
}