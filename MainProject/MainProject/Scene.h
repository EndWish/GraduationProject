#pragma once

#include "GameObject.h"
#include "Camera.h"

class Scene {
protected:	// ��� ������

public:		// ����, �Ҹ���� ��� �Լ���
	// ������, �Ҹ���, �������, �̵�����, �����Ҵ�, �̵��Ҵ�
	Scene();	// �⺻ ������
	virtual ~Scene();	// �Ҹ���

	// �� ��ȯ�� ���õ� �Լ�
	virtual void Enter();	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)
	virtual void Exit();	// ���� ���� ����Ǵ� �Լ� (==OnDestroy)
	virtual void Pause();	// �Ͻ�����
	virtual void Resume();	// �簳

public:		// ��� �Լ���

	// ���� ���࿡ ���õ� �Լ�
	virtual void FrameAdvance() = 0;	// ���� ���������� ���� (== Update, == Run)
	virtual void Render(const ComPtr <ID3D12GraphicsCommandList>& pCommandList) = 0;

	// ���� ������Ʈ �߰� �� �����ϴ� �Լ�
	virtual void AddGameObject(const shared_ptr<GameObject>& pGameObject) = 0;
	virtual void DeleteGameObject(const shared_ptr<GameObject>& pGameObject) = 0;

};

