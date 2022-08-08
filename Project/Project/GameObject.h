#pragma once
class GameObject : public enable_shared_from_this<GameObject> {
protected:
	string name;
	
	// 월드좌표계 기준 : eachTransform 이 바뀌면 항상 동기화 해준다.
	XMFLOAT4X4 worldTransform;

	// 부모좌표계 기준
	XMFLOAT4X4 eachTransform;

	BoundingOrientedBox boundingBox;
	// true일경우 하위 오브젝트들을 모두 포함하는 바운딩박스 객체임
	bool isOOBBBCover;

	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;

public:
	GameObject();
	virtual ~GameObject();

// get set 함수
	
	// 오른쪽 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetRightUnitVector() const;
	// 위쪽 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetUpUnitVector() const;
	// 바라보고 있는 방향의 단위벡터를 얻는다.
	XMFLOAT3 GetLookUnitVector() const;
	// 부모좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetEachPosition() const;
	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const;

	// 자식을 추가한다.
	void SetChild(const shared_ptr<GameObject> pChild);
	// eachTransform의 위치값을 바꾼다.
	void SetEachPosition(const XMFLOAT3& _position);

	// eachTransform를 가지고 worldTransform를 업데이트 한다.
	void UpdateWorldTransform();	
	// 앞으로 이동 (_distance가 음수일 경우 뒤로 이동)
	void MoveFront(float _distance);
	// 오른쪽으로 이동(_distance가 음수일 경우 왼쪽으로 이동)
	void MoveRight(float _distance);
	// Y축을 기준으로 회전
	
	virtual void Animate(double _timeElapsed);

};

