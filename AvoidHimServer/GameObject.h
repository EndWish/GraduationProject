#pragma once

class GameObject {

protected:
	string name;
	ObjectType type;
	UINT id;

	// 변환 행렬, 위치, 회전, 크기
	XMFLOAT4X4 worldTransform;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotation;

	// 바운딩 박스
	BoundingOrientedBox modelBouningBox;	// 모델의 바운딩 박스 (모델 좌표계)
	BoundingOrientedBox worldBouningBox;	// 바운딩 박스 (월드 좌표계)

public:
	GameObject();
	GameObject(const GameObject& _other);	// 복사생성자
	GameObject& operator=(const GameObject& _other);	// 복사 대입 연산자
	virtual ~GameObject();
	
	virtual void Create();
	virtual void Create(const string& _ObjectName);

	const string& GetName() const { return name; }
	ObjectType GetType() const { return type; }
	void SetType(ObjectType _objectType) { type = _objectType; }
	UINT GetID() const { return id; }
	void SetID(UINT _id) { id = _id; }

	// 부모좌표계기준 벡터들을 얻는다.
	XMFLOAT3 GetRightVector() const;
	XMFLOAT3 GetUpVector() const;
	XMFLOAT3 GetLookVector() const;

	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const { return worldBouningBox; }
	
	void SetPosition(const XMFLOAT3& _position) { position = _position; }
	XMFLOAT3 GetPosition() const { return position; }
	void SetRotation(const XMFLOAT4& _rotation) { rotation = _rotation; }
	XMFLOAT4 GetRotation() const { return rotation; }
	void SetScale(const XMFLOAT3& _scale) { scale = _scale; };
	XMFLOAT3 GetScale() const { return scale; }

	// 월드 변환 행렬 업데이트
	void UpdateWorldTransform();

	//  OOBB 갱신
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
	Door& operator=(const Door& _other);	// 복사 대입 연산자
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
	WaterDispenser& operator=(const WaterDispenser& _other);	// 복사 대입 연산자
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
	Lever& operator=(const Lever& _other);	// 복사 대입 연산자
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
	Computer& operator=(const Computer& _other);	// 복사 대입 연산자
	virtual ~Computer();

	bool GetPower() { return power; }
	void SetPower(bool _power) { power = _power; }
	float GetHackingRate() { return hackingRate; }
	void SetHackingRate(float _hackingRate) { hackingRate = _hackingRate; }
	bool GetUse() { return use; }
	void SetUse(bool _use) { use = _use; }

};
