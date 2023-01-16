#pragma once

class GameObject {

protected:
	string name;

	// 변환 행렬, 위치, 회전, 크기
	XMFLOAT4X4 worldTransform;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotation;

	// 바운딩 박스
	bool isOOBBCover;
	BoundingOrientedBox modelBouningBox;	// 모델의 바운딩 박스 (모델 좌표계)
	BoundingOrientedBox worldBouningBox;	// 바운딩 박스 (월드 좌표계)

public:
	GameObject();
	virtual ~GameObject();
	// 게임 오브젝트 복사 생성자
	
	virtual void Create();
	virtual void Create(const string& _ObjectName);

	const string& GetName() const { return name; }

	// 부모좌표계기준 벡터들을 얻는다.
	XMFLOAT3 GetRightVector() const;
	XMFLOAT3 GetUpVector() const;
	XMFLOAT3 GetLookVector() const;
	XMFLOAT4 GetLocalRotate() const { return rotation; }
	XMFLOAT3 GetLocalPosition() const { return position; }

	// 로컬 이동
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveRight(float distance);
	void MoveUp(float distance);
	void MoveFront(float distance);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);

	// 월드좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetWorldPosition() const { return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43); }
	XMFLOAT3 GetWorldRightVector() const { return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13); }
	XMFLOAT3 GetWorldUpVector() const { return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);  }
	XMFLOAT3 GetWorldLookVector() const { return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33); }

	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const { return worldBouningBox; }
	
	void SetLocalPosition(const XMFLOAT3& _position) { position = _position; }
	void SetLocalRotation(const XMFLOAT4& _rotation) { rotation = _rotation; }
	void SetLocalScale(const XMFLOAT3& _scale) { scale = _scale; };

	// 해당 오브젝트 프레임을 최상위 부모 ( 바운딩박스 커버 )로 설정
	void SetOOBBCover(bool _isCover) { isOOBBCover = _isCover; } ;

	// 월드 변환 행렬 업데이트
	void UpdateWorldTransform();

	//  OOBB 갱신
	void UpdateOOBB();

	bool CheckCollision(GameObject* pGameObject);

	
};

