#include "stdafx.h"
#include "GameObject.h"
#include "GameFramework.h"	// ������ü�� �����Ͽ� Scene�� ������Ʈ�� �����ϱ� ���� �����Ͽ���.

/// �������� ��� �Լ���
// ������, �Ҹ���
GameObject::GameObject() 
{
	m_name = "�̸�����";	// ��ü�� �̸�

	Matrix4x4::Identity(m_worldTransform);	// ��->���� ��ǥ�� �ٲٱ� ���� ���
	Matrix4x4::Identity(m_eachTransform);	// �θ� ��ǥ �������� �ڽ��� ��ȯ�ϴ� ���

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// �Ž� ������ �ʱ�ȭ
	// ���׸��� �����͵� �ʱ�ȭ
}
GameObject::~GameObject() 
{
	
}
GameObject::GameObject(const GameObject& other)	//���������
{
	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	// �������� �θ� ���� ��� ���� �θ� �������� �Ѵ�. (�θ� nullptr�ϰ�� �˾Ƽ� Scene�� ����ǵ��� �ϸ� ������ �ϴ�[����])
	if (auto pOtherParent = other.m_pParent.lock())
		pOtherParent->SetChild(shared_from_this());
	// �θ� ���� ��� Scene ���� �����Ѵ�.
	else
		GameFramework::Instance().GetCurrentSceneRef().AddGameObject(shared_from_this());

	//�ڽ� ������Ʈ�� ����
	for (const shared_ptr<const GameObject>& pOtherChild : other.m_pChildren) {
		shared_ptr<GameObject> pMyChild(new GameObject(*pOtherChild));
		SetChild(pMyChild);
	}
}
GameObject::GameObject(GameObject&& other) noexcept 	//�̵�������
{
	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	m_pChildren = move(other.m_pChildren);	//������ �ڽĵ��� �����´�. (������ �ִ� �ڽĵ��� �������.)
	for (shared_ptr<GameObject>& m_pChild : m_pChildren)	// �ڽĵ��� �θ� ���� �ٲ۴�.
		m_pChild->m_pParent = shared_from_this();

	// �θ��� �Ƶ��� ���� ����
	m_pParent = other.m_pParent;
	if (auto pParent = other.m_pParent.lock())	// other�� �θ� �ִٸ�
		*ranges::find(pParent->m_pChildren, other.shared_from_this()) = shared_from_this();	// �θ��� �ڽ��� ��(this)�� ����
	// �θ� ���� ��� Scene���� ���� ������ ������ �Ѵ�.
	else {
		Scene& scene = GameFramework::Instance().GetCurrentSceneRef();
		scene.DeleteGameObject(other.shared_from_this());
		scene.AddGameObject(shared_from_this());
	}
	//���� ������ ���� ����ī��Ʈ�� 0�� �Ǿ� other�� �Ҹ�� ���̴�.

}
GameObject& GameObject::operator=(const GameObject& other) 	//�����Ҵ�
{
	if (this == &other)
		return *this;

	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	// �������� �θ� ���� ��� ���� �θ� �������� �Ѵ�. (�θ� nullptr�ϰ�� �˾Ƽ� Scene�� ����ǵ��� �ϸ� ������ �ϴ�[����])
	if (auto pOtherParent = other.m_pParent.lock())
		pOtherParent->SetChild(shared_from_this());
	// �θ� ���� ��� Scene ���� �����Ѵ�.
	else
		GameFramework::Instance().GetCurrentSceneRef().AddGameObject(shared_from_this());

	//�ڽ� ������Ʈ�� ����
	m_pChildren.clear();
	for (const shared_ptr<const GameObject>& pOtherChild : other.m_pChildren) {
		shared_ptr<GameObject> pMyChild(new GameObject(*pOtherChild));
		SetChild(pMyChild);
	}

	return *this;
}
GameObject& GameObject::operator=(GameObject&& other) noexcept 	//�̵��Ҵ�
{
	if (this == &other)
		return *this;

	m_name = other.m_name;

	m_worldTransform = other.m_worldTransform;
	m_eachTransform = other.m_eachTransform;

	m_OOBB = other.m_OOBB;
	m_isOOBBCover = other.m_isOOBBCover;

	m_pChildren = move(other.m_pChildren);	//������ �ڽĵ��� �����´�. (������ �ִ� �ڽĵ��� �������.)
	for (shared_ptr<GameObject>& m_pChild : m_pChildren)	// �ڽĵ��� �θ� ���� �ٲ۴�.
		m_pChild->m_pParent = shared_from_this();

	// �θ��� �Ƶ��� ���� ����
	m_pParent = other.m_pParent;
	if (auto pParent = other.m_pParent.lock())	// other�� �θ� �ִٸ�
		*ranges::find(pParent->m_pChildren, other.shared_from_this()) = shared_from_this();	// �θ��� �ڽ��� ��(this)�� ����
	// �θ� ���� ��� Scene���� ���� ������ ������ �Ѵ�.
	else {
		Scene& scene = GameFramework::Instance().GetCurrentSceneRef();
		scene.DeleteGameObject(other.shared_from_this());
		scene.AddGameObject(shared_from_this());
	}
	//���� ������ ���� ����ī��Ʈ�� 0�� �Ǿ� other�� �Ҹ�� ���̴�.

	return *this;
}

/// ��� �Լ���
// �ڽ� ������Ʈ �߰�
void GameObject::SetChild(const shared_ptr<GameObject>& pChild)
{
	// �Ծ��� ���̰�, �θ� ���� ��� �θ�� ���� ������Ų��.
	if (auto pParent = pChild->m_pParent.lock())	// �θ� ������
		pParent->m_pChildren.erase( ranges::find(pParent->m_pChildren, pChild) );	// �� �θ𿡼� �ڽ��� ã�� �����Ѵ�.

	// ���� �ڽ����� �Ծ�
	m_pChildren.push_back(pChild);

	// �ڽ��� �θ� ���� ����
	pChild->m_pParent = shared_from_this();
}