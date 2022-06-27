#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject() 
{
	m_nReferenced = 0;	// 이 객체를 참조하고 있는 수

	m_name = "이름없음";	// 객체의 이름

	m_destroyed = false;;	// 해당 객체가 파괴됬는지 확인

	Matrix4x4::Identity(m_worldTransform);	// 모델->월드 좌표로 바꾸기 위한 행렬
	Matrix4x4::Identity(m_eachTransform);	// 부모 좌표 기준으로 자신을 변환하는 행렬

	GameObject* m_pParent = nullptr;	// 부모에 대한 포인터, nullptr일 경우 부모가 없다.
	GameObject* m_pChild = nullptr;		// 자식에 대한 포인터
	GameObject* m_pSibling = nullptr;	// 형제에 대한 포인터

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// 매쉬 포인터 초기화
	// 메테리얼 포인터들 초기화

}
