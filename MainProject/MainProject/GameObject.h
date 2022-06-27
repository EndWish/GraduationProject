#pragma once

class GameObject {
protected:
	int m_nReferenced;	// 이 객체를 참조하고 있는 수

	string m_name;	// 객체의 이름

	bool m_destroyed;	// 해당 객체가 파괴됬는지 확인

	XMFLOAT4X4 m_worldTransform;	// 모델->월드 좌표로 바꾸기 위한 행렬
	XMFLOAT4X4 m_eachTransform;		// 부모 좌표 기준으로 자신을 변환하는 행렬

	GameObject* m_pParent = nullptr;	// 부모에 대한 포인터, nullptr일 경우 부모가 없다.
	GameObject* m_pChild = nullptr;		// 자식에 대한 포인터
	GameObject* m_pSibling = nullptr;	// 형제에 대한 포인터

	BoundingOrientedBox	m_OOBB;	// Object-Oriented Bounding Box
	bool m_isOOBBCover;	// true일 경우 m_OOBB가 자식들의 OOBB들의 대표로 충돌하는지 체크한다.(== 충돌하지 않는다면 하위 객체의 OOBB들도 전부 충돌x로 간주한다.)

	// 매쉬 포인터
	// 메테리얼 포인터들 초기화

public:
	GameObject();

};

