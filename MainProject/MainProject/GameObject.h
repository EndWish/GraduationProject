#pragma once
class GameObject : public enable_shared_from_this<GameObject> {
protected:	// 멤버 변수▼
	string m_name;	// 객체의 이름

	XMFLOAT4X4 m_worldTransform;	// 모델->월드 좌표로 바꾸기 위한 행렬
	XMFLOAT4X4 m_eachTransform;		// 부모 좌표 기준으로 자신을 변환하는 행렬

	BoundingOrientedBox	m_OOBB;	// Object-Oriented Bounding Box
	bool m_isOOBBCover;	// true일 경우 m_OOBB가 자식들의 OOBB들의 대표로 충돌하는지 체크한다.(== 충돌하지 않는다면 하위 객체의 OOBB들도 전부 충돌x로 간주한다.)

	// 매쉬 포인터
	// 메테리얼 포인터들 초기화

	// https://modoocode.com/252 : 스마트 포인터를 완벽히 이해하고 있지 않다면 반드시 읽자.
	weak_ptr<GameObject> m_pParent;	// 부모를 가리키는 포인터
	vector<shared_ptr<GameObject>> m_pChildren;	// 자식들을 가리키는 포인터들 (자신이 root일 경우 Scene에서 포인터로 가리켜 생존시킨다.)

public:		// 생성관련 멤버 함수▼
	// 생성자, 소멸자, 복사생성자, 이동생성자, 복사할당, 이동할당
	GameObject();
	virtual ~GameObject();
	GameObject(const GameObject& other);	//복사생성자 (*자식 오브젝트도 함께 깊은복사한다. *부모가 없을때 Scene에 직접 연결해줘야 한다.)
	GameObject(GameObject&& other) noexcept;	//이동생성자 (이 함수를 호출한 후 other을 사용 하면 안된다. other이 소멸되기 때문에, 또한 other에게 연결되어 있던 포인터도 모두 끊기기 때문에 사용시 각별한 주의가 필요하다.)
	virtual GameObject& operator=(const GameObject& other);	//복사할당 (*자식 오브젝트도 함께 깊은복사한다. *부모가 없을때 Scene에 직접 연결해줘야 한다.)
	virtual GameObject& operator=(GameObject&& other) noexcept;	//이동할당

public:		// 멤버 함수▼
	void SetChild(const shared_ptr<GameObject>& pChild);

};

