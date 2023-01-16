#pragma once

class GameObject {

protected:
	string name;

	// ��ȯ ���, ��ġ, ȸ��, ũ��
	XMFLOAT4X4 worldTransform;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotation;

	// �ٿ�� �ڽ�
	bool isOOBBCover;
	BoundingOrientedBox modelBouningBox;	// ���� �ٿ�� �ڽ� (�� ��ǥ��)
	BoundingOrientedBox worldBouningBox;	// �ٿ�� �ڽ� (���� ��ǥ��)

public:
	GameObject();
	virtual ~GameObject();
	// ���� ������Ʈ ���� ������
	
	virtual void Create();
	virtual void Create(const string& _ObjectName);

	const string& GetName() const { return name; }

	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetRightVector() const;
	XMFLOAT3 GetUpVector() const;
	XMFLOAT3 GetLookVector() const;
	XMFLOAT4 GetLocalRotate() const { return rotation; }
	XMFLOAT3 GetLocalPosition() const { return position; }

	// ���� �̵�
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveRight(float distance);
	void MoveUp(float distance);
	void MoveFront(float distance);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);

	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const { return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43); }
	XMFLOAT3 GetWorldRightVector() const { return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13); }
	XMFLOAT3 GetWorldUpVector() const { return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);  }
	XMFLOAT3 GetWorldLookVector() const { return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33); }

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const { return worldBouningBox; }
	
	void SetLocalPosition(const XMFLOAT3& _position) { position = _position; }
	void SetLocalRotation(const XMFLOAT4& _rotation) { rotation = _rotation; }
	void SetLocalScale(const XMFLOAT3& _scale) { scale = _scale; };

	// �ش� ������Ʈ �������� �ֻ��� �θ� ( �ٿ���ڽ� Ŀ�� )�� ����
	void SetOOBBCover(bool _isCover) { isOOBBCover = _isCover; } ;

	// ���� ��ȯ ��� ������Ʈ
	void UpdateWorldTransform();

	//  OOBB ����
	void UpdateOOBB();

	bool CheckCollision(GameObject* pGameObject);

	
};

