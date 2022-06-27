#include "stdafx.h"
#include "GameObject.h"

// 생성자, 소멸자
GameObject::GameObject() 
{
	m_nReferenced = 0;	// 이 객체를 참조하고 있는 수 (0이 될때 delete를 이용하여 제거 되도록 한다.)
	m_destroyed = false;;	// 해당 객체가 파괴됬는지 확인 (true일 경우 참조하는 쪽에서 참조를 끊는다. => 끊어지다 보면 m_nReferenced가 0이 되어 메모리가 반환될 것이다. + true가 될때 내가 참조하고 있는 포인터들을 전부 끊는다.)

	m_name = "이름없음";	// 객체의 이름

	Matrix4x4::Identity(m_worldTransform);	// 모델->월드 좌표로 바꾸기 위한 행렬
	Matrix4x4::Identity(m_eachTransform);	// 부모 좌표 기준으로 자신을 변환하는 행렬

	m_pParent = nullptr;	// 부모에 대한 포인터, nullptr일 경우 부모가 없다.
	m_pChild = nullptr;		// 자식에 대한 포인터
	m_pSibling = nullptr;	// 형제에 대한 포인터

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// 매쉬 포인터 초기화
	// 메테리얼 포인터들 초기화

}
GameObject::~GameObject() {
	
}