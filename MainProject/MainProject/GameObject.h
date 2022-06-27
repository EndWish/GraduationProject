#pragma once

class GameObject {
protected:
	int m_nReferenced;	// �� ��ü�� �����ϰ� �ִ� ��

	string m_name;	// ��ü�� �̸�

	bool m_destroyed;	// �ش� ��ü�� �ı������ Ȯ��

	XMFLOAT4X4 m_worldTransform;	// ��->���� ��ǥ�� �ٲٱ� ���� ���
	XMFLOAT4X4 m_eachTransform;		// �θ� ��ǥ �������� �ڽ��� ��ȯ�ϴ� ���

	GameObject* m_pParent = nullptr;	// �θ� ���� ������, nullptr�� ��� �θ� ����.
	GameObject* m_pChild = nullptr;		// �ڽĿ� ���� ������
	GameObject* m_pSibling = nullptr;	// ������ ���� ������

	BoundingOrientedBox	m_OOBB;	// Object-Oriented Bounding Box
	bool m_isOOBBCover;	// true�� ��� m_OOBB�� �ڽĵ��� OOBB���� ��ǥ�� �浹�ϴ��� üũ�Ѵ�.(== �浹���� �ʴ´ٸ� ���� ��ü�� OOBB�鵵 ���� �浹x�� �����Ѵ�.)

	// �Ž� ������
	// ���׸��� �����͵� �ʱ�ȭ

public:
	GameObject();

};

