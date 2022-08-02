#pragma once

#include "Scene.h"

class ScenePlay : public Scene {
protected:	// ��� ������
	weak_ptr<Camera> m_pMainCamera;


public:		// �������� ��� �Լ���
	ScenePlay();
	virtual ~ScenePlay();

	// �� ��ȯ�� ���õ� �Լ�
	virtual void Enter();	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)

public:		// ��� �Լ���
	
	// ���� ���࿡ ���õ� �Լ�
	virtual void FrameAdvance();	// ���� ���������� ���� (== Update, == Run)
	virtual void Render(const ComPtr <ID3D12GraphicsCommandList>& pCommandList);

	// ���� ������Ʈ �߰� �� �����ϴ� �Լ�
	virtual void AddGameObject(const shared_ptr<GameObject>& pGameObject);
	virtual void DeleteGameObject(const shared_ptr<GameObject>& pGameObject);

};

