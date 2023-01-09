#pragma once
#include "GameObject.h"

struct VS_CameraMappedFormat {
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT3 position;
	float padding;
	XMFLOAT3 playerPosition;	// ���� �÷��̾��� ��ġ

};

class Camera : public GameObject {
protected:
	XMFLOAT4X4 viewTransform;
	XMFLOAT4X4 projectionTransform;

	XMFLOAT3 playerPosition;
	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	ComPtr<ID3D12Resource> pCameraBuffer;
	shared_ptr<VS_CameraMappedFormat> pMappedCamera;

public:
	Camera();
	virtual ~Camera();

	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	void SetViewPortAndScissorRect(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateViewTransform();
	void UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle);
	void SetPlayerPos(XMFLOAT3 _playerPos) { playerPosition = _playerPos; };
	virtual void UpdateWorldTransform();
};
