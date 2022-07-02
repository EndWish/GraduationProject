#pragma once

#include "GameObject.h"

struct VS_CB_CAMERA_INFO {
	XMFLOAT4X4 m_viewTransform;
	XMFLOAT4X4 m_projectionTransform;
};

class Camera : public GameObject {
protected:	// ��� ������
	XMFLOAT3 m_lookAtWorld;	// �ٶ� ����
	XMFLOAT3 m_offset;		// �󸶳� �������� ������ ���� ��

	XMFLOAT4X4 m_viewTransform;				// ī�޶��� �������� ���带 �ٶ󺸵��� ���ִ� ��ȯ ���
	XMFLOAT4X4 m_projectionTransform;		// ���� ���� ���, �ָ��ִ� ���� �۰� �������� ũ�� ���̰� ���ش�.

	D3D12_VIEWPORT m_viewport;	// (��,��,��,��,�����Ÿ�,�����Ÿ�) ������ �Ҷ� ����Ÿ��(�ĸ����) ������ ��Ÿ���� ����ü, ��� ����Ʈ�� Reset() �ɶ����� ����Ʈ�� �ٽ� ������ ����Ѵ�. ����Ʈ�� ������ 1���� ũ�� ���� ���̴����� SV_ViewportArrayIndex�� ���, ��� ����Ʈ�� ���ÿ� �����ؾ��Ѵ�.?
	D3D12_RECT m_scissorRect;	// ���������� ���ܵ��� ���� ����(=�ٽ� �׸� ����), �׿� ����Ʈ�� ����

	ComPtr<ID3D12Resource> m_pCameraTransformBuffer;
	VS_CB_CAMERA_INFO* m_pMappedCameraTransformBuffer;	// ������ ���ҽ��� ���� CPU �����͸� �������� GPU���� ������ ��� ���� ������ �ݿ��ϵǷ� ���ش�.

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Camera(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList);
	virtual ~Camera();

	// ī�޶� ��ȯ(��ġ, �亯ȯ ���, ������ȯ ���)�� ������ ��� ���ҽ��� �Ҵ��Ѵ�.
	virtual void CreateShaderVariables(const ComPtr<ID3D12Device>& pDevice, const ComPtr<ID3D12GraphicsCommandList>& pCommandList);	// ī�޶� ��ȯ�� �ʿ��� �������� ���� ���ε����(���ҽ�) ���� �� ����

public:		// ��� �Լ���
	//get set �Լ�
	virtual void SetViewportsAndScissorRects(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);
	void SetOffset(const XMFLOAT3& offset);


	// ��ĺ�ȯ ���� �Լ�
	void GenerateViewMatrix();	// ������ ��ġ, ���� ��ġ, �����ͷ� �亯ȯ ����� ���� ����Ѵ�.
	void GenerateViewMatrix(const XMFLOAT3& lookAtWorld);
	void GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float FOVAngle);
	virtual void UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);

	// ī�޶�� ������ ���� �ʵ��� �������̵� ���� [����]

};

