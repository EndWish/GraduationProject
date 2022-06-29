#include "stdafx.h"
#include "Scene.h"

// 생성자, 소멸자, 복사생성, 이동생성, 복사할당, 이동할당
Scene::Scene() // 기본 생성자
{

}
Scene::~Scene() // 소멸자
{

}

// 씬 전환에 관련된 함수
void Scene::Enter()	// 게임프레임워크에 들어가면서 초기화시키는 함수 (==OnCreate)
{

}
void Scene::Exit() // 씬이 삭제 수행되는 함수 (==OnDestroy)
{

}
void Scene::Pause() // 일시정지
{

}
void Scene::Resume()	// 재개
{

}

// 게임 진행에 관련된 함수
void Scene::FrameAdvance()	// 다음 프레임으로 진행 (== Update, == Run)
{
	// 입력처리
	// 오브젝트들 애니메이션 처리
	// 충돌처리
	
	// 렌더링은 프레임워크에서 추가적인 설정을 한후 씬의 Render()를 따로 부를 것
}