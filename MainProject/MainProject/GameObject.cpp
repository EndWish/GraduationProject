#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject() 
{
	m_nReferenced = 0;	// �� ��ü�� �����ϰ� �ִ� ��

	m_name = "�̸�����";	// ��ü�� �̸�

	m_destroyed = false;;	// �ش� ��ü�� �ı������ Ȯ��

	Matrix4x4::Identity(m_worldTransform);	// ��->���� ��ǥ�� �ٲٱ� ���� ���
	Matrix4x4::Identity(m_eachTransform);	// �θ� ��ǥ �������� �ڽ��� ��ȯ�ϴ� ���

	GameObject* m_pParent = nullptr;	// �θ� ���� ������, nullptr�� ��� �θ� ����.
	GameObject* m_pChild = nullptr;		// �ڽĿ� ���� ������
	GameObject* m_pSibling = nullptr;	// ������ ���� ������

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// �Ž� ������ �ʱ�ȭ
	// ���׸��� �����͵� �ʱ�ȭ

}
