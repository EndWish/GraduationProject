#include "stdafx.h"
#include "GameObject.h"

// ������, �Ҹ���
GameObject::GameObject() 
{
	m_nReferenced = 0;	// �� ��ü�� �����ϰ� �ִ� �� (0�� �ɶ� delete�� �̿��Ͽ� ���� �ǵ��� �Ѵ�.)
	m_destroyed = false;;	// �ش� ��ü�� �ı������ Ȯ�� (true�� ��� �����ϴ� �ʿ��� ������ ���´�. => �������� ���� m_nReferenced�� 0�� �Ǿ� �޸𸮰� ��ȯ�� ���̴�. + true�� �ɶ� ���� �����ϰ� �ִ� �����͵��� ���� ���´�.)

	m_name = "�̸�����";	// ��ü�� �̸�

	Matrix4x4::Identity(m_worldTransform);	// ��->���� ��ǥ�� �ٲٱ� ���� ���
	Matrix4x4::Identity(m_eachTransform);	// �θ� ��ǥ �������� �ڽ��� ��ȯ�ϴ� ���

	m_pParent = nullptr;	// �θ� ���� ������, nullptr�� ��� �θ� ����.
	m_pChild = nullptr;		// �ڽĿ� ���� ������
	m_pSibling = nullptr;	// ������ ���� ������

	m_OOBB = BoundingOrientedBox();
	m_isOOBBCover = false;

	// �Ž� ������ �ʱ�ȭ
	// ���׸��� �����͵� �ʱ�ȭ

}
GameObject::~GameObject() {
	
}