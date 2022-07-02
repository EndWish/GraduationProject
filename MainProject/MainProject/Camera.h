#pragma once

#include "GameObject.h"

struct VS_CB_CAMERA_INFO {
	XMFLOAT4X4 m_viewTransform;
	XMFLOAT4X4 m_projectionTransform;
};

class Camera : public GameObject {
protected:	// 멤버 변수▼
	XMFLOAT3 m_lookAtWorld;	// 바라볼 방향
	XMFLOAT3 m_offset;		// 얼마나 떨어져서 볼지에 대한 값

	XMFLOAT4X4 m_viewTransform;				// 카메라의 시점으로 월드를 바라보도록 해주는 변환 행렬
	XMFLOAT4X4 m_projectionTransform;		// 원근 투영 행렬, 멀리있는 것은 작게 가까운것은 크게 보이게 해준다.

	D3D12_VIEWPORT m_viewport;	// (좌,상,우,하,근평면거리,원평면거리) 렌더링 할때 렌더타겟(후면버퍼) 영역을 나타내는 구조체, 명령 리스트가 Reset() 될때마다 뷰포트를 다시 설정해 줘야한다. 뷰포트의 개수가 1보다 크면 기하 쉐이더에서 SV_ViewportArrayIndex를 사용, 모든 뷰포트를 동시에 설정해야한다.?
	D3D12_RECT m_scissorRect;	// 레더링에서 제외되지 않을 영역(=다시 그릴 영역), 그외 뷰포트와 동일

	ComPtr<ID3D12Resource> m_pCameraTransformBuffer;
	VS_CB_CAMERA_INFO* m_pMappedCameraTransformBuffer;	// 매핑은 리소스에 대한 CPU 포인터를 가져오고 GPU에서 수행한 모든 수정 사항을 반영하되록 해준다.

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Camera(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList);
	virtual ~Camera();

	// 카메라 변환(위치, 뷰변환 행렬, 투영변환 행렬)의 정보를 담는 리소스를 할당한다.
	virtual void CreateShaderVariables(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList);	// 카메라 변환에 필요한 변수들을 담을 업로드버퍼(리소스) 생성 및 매핑

public:		// 멤버 함수▼
	//get set 함수
	virtual void SetViewportsAndScissorRects(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);
	void SetOffset(const XMFLOAT3& offset);


	// 행렬변환 관련 함수
	void GenerateViewMatrix();	// 현재의 위치, 보라볼 위치, 업벡터로 뷰변환 행렬을 새로 계산한다.
	void GenerateViewMatrix(const XMFLOAT3& lookAtWorld);
	void GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float FOVAngle);
	virtual void UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);

	// 카메라는 렌더링 하지 않도록 오버라이딩 하자 [수정]

};

