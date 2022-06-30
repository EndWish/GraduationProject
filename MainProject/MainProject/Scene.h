#pragma once

#include "GameObject.h"

class Scene {


public:
	// ������, �Ҹ���, �������, �̵�����, �����Ҵ�, �̵��Ҵ�
	Scene();	// �⺻ ������
	virtual ~Scene();	// �Ҹ���

	// �� ��ȯ�� ���õ� �Լ�
	virtual void Enter();	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)
	virtual void Exit();	// ���� ���� ����Ǵ� �Լ� (==OnDestroy)
	virtual void Pause();	// �Ͻ�����
	virtual void Resume();	// �簳

	// ���� ���࿡ ���õ� �Լ�
	virtual void FrameAdvance();	// ���� ���������� ���� (== Update, == Run)

	// ���� ������Ʈ �߰� �� �����ϴ� �Լ�
	virtual void AddGameObject(const shared_ptr<GameObject>& pGameObject) = 0;
	virtual void DeleteGameObject(const shared_ptr<GameObject>& pGameObject) = 0;

};

