#include "stdafx.h"
#include "ScenePlay.h"

/// 생성관련 멤버 함수▼
// 생성자, 소멸자
ScenePlay::ScenePlay() 
{

}
ScenePlay::~ScenePlay() 
{

}

/// 멤버 함수▼
// 씬 전환에 관련된 함수
void ScenePlay::Enter()	// 게임프레임워크에 들어가면서 초기화시키는 함수 (==OnCreate)
{
#ifdef DEBUG
	cout << "Scene build complete!\n";
#endif // DEBUG

}

// 게임 진행에 관련된 함수
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

// 씬에 오브젝트 추가 및 삭제하는 함수
void ScenePlay::AddGameObject(const shared_ptr<GameObject>& pGameObject)
{

}
void ScenePlay::DeleteGameObject(const shared_ptr<GameObject>& pGameObject)
{

}
