#include "stdafx.h"
#include "Scene.h"

// �������� ��� �Լ���
// ������, �Ҹ���
Scene::Scene() // �⺻ ������
{

}
Scene::~Scene() // �Ҹ���
{

}

// ��� �Լ���
// �� ��ȯ�� ���õ� �Լ�
void Scene::Enter()	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)
{

}
void Scene::Exit() // ���� ���� ����Ǵ� �Լ� (==OnDestroy)
{

}
void Scene::Pause() // �Ͻ�����
{

}
void Scene::Resume()	// �簳
{

}

// ���� ���࿡ ���õ� �Լ�
void Scene::FrameAdvance()	// ���� ���������� ���� (== Update, == Run)
{
	// �Է�ó��
	// ������Ʈ�� �ִϸ��̼� ó��
	// �浹ó��
	
	// �������� �����ӿ�ũ���� �߰����� ������ ���� ���� Render()�� ���� �θ� ��
}