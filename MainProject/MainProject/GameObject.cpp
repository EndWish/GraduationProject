#include "stdafx.h"
#include "GameObject.h"
#include "GameFramework.h"	// 상위객체에 접근하여 Scene에 오브젝트를 연결하기 위해 포함하였다.

/// 생성관련 멤버 함수▼
// 생성자, 소멸자
GameObject::GameObject() 
{
	m_name = "이름없음";	// 객체의 이름

	Matrix4x4::Identity(m_worldTransform);	// 모델->월드 좌표로 바꾸기 위한 행렬
	Matrix4x4::Identity(m_eachTransform);	// 부모 좌표 기준으로 자신을 변환하는 행렬

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// 매쉬 포인터 초기화
	// 메테리얼 포인터들 초기화
}
GameObject::~GameObject() 
{
	
}
GameObject::GameObject(const GameObject& other)	//복사생성자
{
	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	// 복사대상이 부모가 있을 경우 같은 부모를 가지도록 한다. (부모가 nullptr일경우 알아서 Scene에 연결되도록 하면 좋을듯 하다[수정])
	if (auto pOtherParent = other.m_pParent.lock())
		pOtherParent->SetChild(shared_from_this());
	// 부모가 없을 경우 Scene 직접 연결한다.
	else
		GameFramework::Instance().GetCurrentSceneRef().AddGameObject(shared_from_this());

	//자식 오브젝트들 복사
	for (const shared_ptr<const GameObject>& pOtherChild : other.m_pChildren) {
		shared_ptr<GameObject> pMyChild(new GameObject(*pOtherChild));
		SetChild(pMyChild);
	}
}
GameObject::GameObject(GameObject&& other) noexcept 	//이동생성자
{
	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	m_pChildren = move(other.m_pChildren);	//상대방의 자식들을 가져온다. (가지고 있던 자식들은 사라진다.)
	for (shared_ptr<GameObject>& m_pChild : m_pChildren)	// 자식들의 부모를 나로 바꾼다.
		m_pChild->m_pParent = shared_from_this();

	// 부모의 아들을 나로 설정
	m_pParent = other.m_pParent;
	if (auto pParent = other.m_pParent.lock())	// other의 부모가 있다면
		*ranges::find(pParent->m_pChildren, other.shared_from_this()) = shared_from_this();	// 부모의 자식을 나(this)로 설정
	// 부모가 없을 경우 Scene에서 직접 삭제와 연결을 한다.
	else {
		Scene& scene = GameFramework::Instance().GetCurrentSceneRef();
		scene.DeleteGameObject(other.shared_from_this());
		scene.AddGameObject(shared_from_this());
	}
	//위의 과정을 통해 참조카운트가 0이 되어 other이 소멸될 것이다.

}
GameObject& GameObject::operator=(const GameObject& other) 	//복사할당
{
	if (this == &other)
		return *this;

	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	// 복사대상이 부모가 있을 경우 같은 부모를 가지도록 한다. (부모가 nullptr일경우 알아서 Scene에 연결되도록 하면 좋을듯 하다[수정])
	if (auto pOtherParent = other.m_pParent.lock())
		pOtherParent->SetChild(shared_from_this());
	// 부모가 없을 경우 Scene 직접 연결한다.
	else
		GameFramework::Instance().GetCurrentSceneRef().AddGameObject(shared_from_this());

	//자식 오브젝트들 복사
	m_pChildren.clear();
	for (const shared_ptr<const GameObject>& pOtherChild : other.m_pChildren) {
		shared_ptr<GameObject> pMyChild(new GameObject(*pOtherChild));
		SetChild(pMyChild);
	}

	return *this;
}
GameObject& GameObject::operator=(GameObject&& other) noexcept 	//이동할당
{
	if (this == &other)
		return *this;

	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	m_pChildren = move(other.m_pChildren);	//상대방의 자식들을 가져온다. (가지고 있던 자식들은 사라진다.)
	for (shared_ptr<GameObject>& m_pChild : m_pChildren)	// 자식들의 부모를 나로 바꾼다.
		m_pChild->m_pParent = shared_from_this();

	// 부모의 아들을 나로 설정
	m_pParent = other.m_pParent;
	if (auto pParent = other.m_pParent.lock())	// other의 부모가 있다면
		*ranges::find(pParent->m_pChildren, other.shared_from_this()) = shared_from_this();	// 부모의 자식을 나(this)로 설정
	// 부모가 없을 경우 Scene에서 직접 삭제와 연결을 한다.
	else {
		Scene& scene = GameFramework::Instance().GetCurrentSceneRef();
		scene.DeleteGameObject(other.shared_from_this());
		scene.AddGameObject(shared_from_this());
	}
	//위의 과정을 통해 참조카운트가 0이 되어 other이 소멸될 것이다.

	return *this;
}

/// 멤버 함수▼
// 자식 오브젝트 추가
void GameObject::SetChild(const shared_ptr<GameObject>& pChild)
{
	// 입양할 아이가, 부모가 있을 경우 부모로 부터 독립시킨다.
	if (auto pParent = pChild->m_pParent.lock())	// 부모가 있으면
		pParent->m_pChildren.erase( ranges::find(pParent->m_pChildren, pChild) );	// 그 부모에서 자식을 찾아 삭제한다.

	// 나의 자식으로 입양
	m_pChildren.push_back(pChild);

	// 자식의 부모를 나로 지정
	pChild->m_pParent = shared_from_this();
}