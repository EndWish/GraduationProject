#pragma once
#include "Mesh.h"

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	string name;
	
	// ������ǥ�� ���� : eachTransform �� �ٲ�� �׻� ����ȭ ���ش�.
	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ� ��ü��
	bool isOOBBBCover;

	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

	weak_ptr<Mesh> pMesh;

public:
	GameObject();
	virtual ~GameObject();

// get set �Լ�
	
	// ������ ������ �������͸� ��´�.
	XMFLOAT3 GetEachRightVector() const;
	// ���� ������ �������͸� ��´�.
	XMFLOAT3 GetEachUpVector() const;
	// �ٶ󺸰� �ִ� ������ �������͸� ��´�.
	XMFLOAT3 GetEachLookVector() const;
	// �θ���ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetEachPosition() const;
	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// ������ �̵��ϴ� ����� ��´�.
	XMFLOAT4X4 GetFrontMoveMatrix(float _distance) const;
	// ������ �̵��ϴ� ����� ��´�.
	XMFLOAT4X4 GetRightMoveMatrix(float _distance) const;
	// Z���� �������� ȸ���ϴ� ����� ��´�.

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

	// ��ġ�� ������ �̵���Ų��.
	void SetEachPosition(XMFLOAT3& _position);
	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> _pChild);

	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.
	void ApplyTransform(XMFLOAT4X4& _transform);

	// �ִϸ��̼�
	virtual void Animate(double _timeElapsed);
	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};

