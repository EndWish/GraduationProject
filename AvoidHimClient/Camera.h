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
	shared_ptr<BoundingFrustum> pBoundingFrustum;
	
	XMFLOAT3 localOffset;

	float minDistance, maxDistance;

public:
	Camera();
	virtual ~Camera();

	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	float GetCurrentDistance() const { return Vector3::Length(XMFLOAT3(localPosition.x, localPosition.y - 1.0f, localPosition.z)); }
	void SetMinDistance(float _minDistance) { minDistance = _minDistance; }
	float GetMinDistance() const { return minDistance; }
	void SetMaxDistance(float _maxDistance) { minDistance = _maxDistance; }
	float GetMaxDistance() const { return maxDistance; }

	void SetViewPortAndScissorRect(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateViewTransform();
	void UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle);
	
	virtual void MoveFront(float distance, float _timeElapsed = 1.0f);

	virtual void UpdateLocalTransform();
	virtual void UpdateWorldTransform();
	virtual void UpdateObject();
	
	const XMFLOAT4X4& GetProjectionTransform() { return projectionTransform; };
	const XMFLOAT4X4& GetViewTransform() { return viewTransform; };
	shared_ptr<BoundingFrustum> GetBoundingFrustum() { return pBoundingFrustum; };

	void SetLocalOffset(const XMFLOAT3& _localOffset) { localOffset = _localOffset; }
	XMFLOAT3 GetLocalOffset() const { return localOffset; }

};
