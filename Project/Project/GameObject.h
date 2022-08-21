#pragma once
#include "Mesh.h"

class Light;

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	// ���� GameObject�� ������ �߰��ߴٸ�, ���� �����ڵ� �����ض�

	string name;
	
	// ������ǥ�� ���� : eachTransform �� �ٲ�� �׻� ����ȭ ���ش�.
	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 eachTransform;

	// ��ü�� ������ �ִ� ���� ������
	shared_ptr<Light> pLight;


	BoundingOrientedBox boundingBox;
	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ� ��ü��
	bool isOOBBBCover;

	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;
	
	weak_ptr<Mesh> pMesh;

public:
	GameObject();
	virtual ~GameObject();
	// ���� ������Ʈ ���� ������
	GameObject(const GameObject& other);

	virtual void Create();
	virtual void Create(string _ObjectName);

// get set �Լ�
	const string& GetName() const;

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
	// ȸ������ �������� ȸ���ϴ� ����� ��´�.
	XMFLOAT4X4 GetRotateMatrix(const XMFLOAT3& _axis, float _angle) const;
	// ���ʹϾ����� ȸ���ϴ� ����� ��´�.
	XMFLOAT4X4 GetRotateMatrix(const XMFLOAT4& _quaternion) const;
	// pitch, yaw, roll ���� ȸ���ϴ� ����� ��´�.
	XMFLOAT4X4 GetRotateMatrix(float _pitch, float _yaw, float _roll) const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

	// ��ġ�� ������ �̵���Ų��.
	void SetEachPosition(const XMFLOAT3& _position);
	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// �޽��� �����Ѵ�.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);

	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.
	void ApplyTransform(const XMFLOAT4X4& _transform);

	// �ִϸ��̼�
	virtual void Animate(double _timeElapsed);
	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	// ���� ��ȯ����� ���̴��� �Ѱ��ش�.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	void LoadFromFile(ifstream& _file);
};


class GameObjectManager {
	map<string, shared_ptr<GameObject>> storage;

public:

	shared_ptr<GameObject> GetGameObject(const string& _name);
};

