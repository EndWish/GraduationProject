#pragma once

class GameObject {

protected:
	string name;
	ObjectType type;
	UINT id;

	// ��ȯ ���, ��ġ, ȸ��, ũ��
	XMFLOAT4X4 worldTransform;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotation;

	// �ٿ�� �ڽ�
	BoundingOrientedBox modelBouningBox;	// ���� �ٿ�� �ڽ� (�� ��ǥ��)
	BoundingOrientedBox worldBouningBox;	// �ٿ�� �ڽ� (���� ��ǥ��)

public:
	GameObject();
	GameObject(const GameObject& _other);	// ���������
	GameObject& operator=(const GameObject& _other);	// ���� ���� ������
	virtual ~GameObject();
	
	virtual void Create();
	virtual void Create(const string& _ObjectName);

	const string& GetName() const { return name; }
	ObjectType GetType() const { return type; }
	void SetType(ObjectType _objectType) { type = _objectType; }
	UINT GetID() const { return id; }
	void SetID(UINT _id) { id = _id; }

	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetRightVector() const;
	XMFLOAT3 GetUpVector() const;
	XMFLOAT3 GetLookVector() const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const { return worldBouningBox; }
	
	void SetPosition(const XMFLOAT3& _position) { position = _position; }
	XMFLOAT3 GetPosition() const { return position; }
	void SetRotation(const XMFLOAT4& _rotation) { rotation = _rotation; }
	XMFLOAT4 GetRotation() const { return rotation; }
	void SetScale(const XMFLOAT3& _scale) { scale = _scale; };
	XMFLOAT3 GetScale() const { return scale; }

	// ���� ��ȯ ��� ������Ʈ
	void UpdateWorldTransform();

	//  OOBB ����
	void UpdateOOBB();

	bool CheckCollision(GameObject* pGameObject);

	
};

