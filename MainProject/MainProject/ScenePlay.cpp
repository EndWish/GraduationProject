#include "stdafx.h"
#include "ScenePlay.h"

/// �������� ��� �Լ���
// ������, �Ҹ���
ScenePlay::ScenePlay() 
{

}
ScenePlay::~ScenePlay() 
{

}

/// ��� �Լ���
// �� ��ȯ�� ���õ� �Լ�
void ScenePlay::Enter()	// ���������ӿ�ũ�� ���鼭 �ʱ�ȭ��Ű�� �Լ� (==OnCreate)
{
#ifdef DEBUG
	cout << "Scene build complete!\n";
#endif // DEBUG

}

// ���� ���࿡ ���õ� �Լ�
void ScenePlay::FrameAdvance()
{
	
}
void ScenePlay::Render(const ComPtr <ID3D12GraphicsCommandList>& pCommandList)
{
	//if (auto pMainCamera = m_pMainCamera.lock()) {
	//	pMainCamera->GenerateViewMatrix();
	//	pMainCamera->SetViewportsAndScissorRects(pCommandList);
	//	pMainCamera->UpdateShaderVariables(pCommandList);
	//}

}

// ���� ������Ʈ �߰� �� �����ϴ� �Լ�
void ScenePlay::AddGameObject(const shared_ptr<GameObject>& pGameObject)
{

}
void ScenePlay::DeleteGameObject(const shared_ptr<GameObject>& pGameObject)
{

}
