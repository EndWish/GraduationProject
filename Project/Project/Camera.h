#pragma once
#include "GameObject.h"

struct VS_CameraMappedFormat {
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT3 position;
};

class Camera : public GameObject {
protected:
	XMFLOAT4X4 viewTransform;
	XMFLOAT4X4 projectionTransform;

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	ComPtr<ID3D12Resource> pCameraBuffer;
	shared_ptr<VS_CameraMappedFormat> pMappedCamera;

public:
	Camera();
	virtual ~Camera();

	void Create();

	void SetViewPortAndScissorRect();
	void UpdateShaderVariable();
	void UpdateViewTransform();
	void UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle);

	virtual void UpdateWorldTransform();
};
