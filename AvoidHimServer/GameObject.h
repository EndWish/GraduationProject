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

	virtual void Animate(float _timeElapsed);
	
};

///////////////////////////////////////////////////////////////////////////////
/// Door

class Door : public GameObject {
protected:
	bool isOpen;

public:
	Door();
	Door(const Door& _other);
	Door& operator=(const Door& _other);	// ���� ���� ������
	virtual ~Door();

	bool IsOpen() { return isOpen; }
	void SetOpen(bool _open) { isOpen = _open; }

};

///////////////////////////////////////////////////////////////////////////////
/// WaterDispenser

class WaterDispenser : public GameObject {
protected:
	float coolTime;

public:
	WaterDispenser();
	WaterDispenser(const WaterDispenser& _other);
	WaterDispenser& operator=(const WaterDispenser& _other);	// ���� ���� ������
	virtual ~WaterDispenser();

	float GetCoolTime() { return coolTime; }
	void SetCoolTime(float _coolTime) { coolTime = _coolTime; }
	void SubtractCoolTime(float _time) { coolTime -= _time; }

	virtual void Animate(float _timeElapsed);

};

///////////////////////////////////////////////////////////////////////////////
/// Lever

class Lever : public GameObject {
protected:
	bool power;

public:
	Lever();
	Lever(const Lever& _other);
	Lever& operator=(const Lever& _other);	// ���� ���� ������
	virtual ~Lever();

	bool GetPower() { return power; }
	void SetPower(bool _power) { power = _power; }

};

///////////////////////////////////////////////////////////////////////////////
/// Computer

class Computer : public GameObject {
protected:
	bool power;
	float hackingRate;
	bool use;

public:
	Computer();
	Computer(const Computer& _other);
	Computer& operator=(const Computer& _other);	// ���� ���� ������
	virtual ~Computer();

	bool GetPower() { return power; }
	void SetPower(bool _power) { power = _power; }
	float GetHackingRate() { return hackingRate; }
	void SetHackingRate(float _hackingRate) { hackingRate = _hackingRate; }
	bool GetUse() { return use; }
	void SetUse(bool _use) { use = _use; }

};
