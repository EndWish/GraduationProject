#pragma once
#include "Mesh.h"


class Light;

class GameObject : public enable_shared_from_this<GameObject> {
protected:
	// ���� GameObject�� ������ �߰��ߴٸ�, ���� �����ڵ� �����ض�

	string name;
public:
	// ������ǥ�� ���� : eachTransform �� �ٲ�� �׻� ����ȭ ���ش�.
	XMFLOAT4X4 worldTransform;

	// �θ���ǥ�� ����
	XMFLOAT4X4 localTransform;
	XMFLOAT3 localPosition;
	XMFLOAT3 localScale;
	XMFLOAT4 localRotation;

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
	//
	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set �Լ�
	const string& GetName() const;
	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT3 GetLocalPosition() const;
	// ���� �̵�
	void MoveRight(float distance);
	void MoveUp(float distance);
	void MoveFront(float distance);
	void Rotate(const XMFLOAT3& _axis, float _angle);
	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

	// ��ġ�� ������ �̵���Ų��.
	void SetLocalPosition(const XMFLOAT3& _position);
	// Ư�� ȸ������ �����Ѵ�.
	void SetLocalRotation(const XMFLOAT4& _rotation);
	// Ư�� Scale���� �����Ѵ�.
	void SetLocalScale(const XMFLOAT3& _scale);


	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// �޽��� �����Ѵ�.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);

	void UpdateLocalTransform();
	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.

	//  OOBB ����
	void UpdateOOBB();

	// ������Ʈ ���� ��ü������ ����
	void UpdateObject();

	// �浹 üũ
	bool CheckCollision(const GameObject& _other);

	// �ִϸ��̼�
	virtual void Animate(double _timeElapsed);
	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	// ���� ��ȯ����� ���̴��� �Ѱ��ش�.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void CopyObject(const GameObject& _other);
};



class GameObjectManager {
	map<string, shared_ptr<GameObject>> storage;

public:

	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

