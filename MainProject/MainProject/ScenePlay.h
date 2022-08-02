#pragma once

#include "Scene.h"

class ScenePlay : public Scene {
protected:	// 멤버 변수▼
	weak_ptr<Camera> m_pMainCamera;


public:		// 생성관련 멤버 함수▼
	ScenePlay();
	virtual ~ScenePlay();

	// 씬 전환에 관련된 함수
	virtual void Enter();	// 게임프레임워크에 들어가면서 초기화시키는 함수 (==OnCreate)

public:		// 멤버 함수▼
	
	// 게임 진행에 관련된 함수
	virtual void FrameAdvance();	// 다음 프레임으로 진행 (== Update, == Run)
	virtual void Render(const ComPtr <ID3D12GraphicsCommandList>& pCommandList);

	// 씬에 오브젝트 추가 및 삭제하는 함수
	virtual void AddGameObject(const shared_ptr<GameObject>& pGameObject);
	virtual void DeleteGameObject(const shared_ptr<GameObject>& pGameObject);

};

