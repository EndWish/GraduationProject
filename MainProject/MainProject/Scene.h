#pragma once

class Scene {


public:
	// ������, �Ҹ���, �������, �̵�����, �����Ҵ�, �̵��Ҵ�
	Scene();	// �⺻ ������
	~Scene();	// �Ҹ���

	// �� ��ȯ�� ���õ� �Լ�
	virtual void Enter();	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)
	virtual void Exit();	// ���� ���� ����Ǵ� �Լ� (==OnDestroy)
	virtual void Pause();	// �Ͻ�����
	virtual void Resume();	// �簳

	// ���� ���࿡ ���õ� �Լ�
	virtual void FrameAdvance();	// ���� ���������� ���� (== Update, == Run)

};

