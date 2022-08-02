#pragma once

#include "GameObject.h"
#include "Camera.h"

class Scene {
protected:	// 멤버 변수▼

public:		// 생성, 소멸관련 멤버 함수▼
	// 생성자, 소멸자, 복사생성, 이동생성, 복사할당, 이동할당
	Scene();	// 기본 생성자
	virtual ~Scene();	// 소멸자

	// 씬 전환에 관련된 함수
	virtual void Enter();	// 게임프레임워크에 들어가면서 초기화시키는 함수 (==OnCreate)
	virtual void Exit();	// 씬이 삭제 수행되는 함수 (==OnDestroy)
	virtual void Pause();	// 일시정지
	virtual void Resume();	// 재개

public:		// 멤버 함수▼

	// 게임 진행에 관련된 함수
	virtual void FrameAdvance() = 0;	// 다음 프레임으로 진행 (== Update, == Run)
	virtual void Render(const ComPtr <ID3D12GraphicsCommandList>& pCommandList) = 0;

	// 씬에 오브젝트 추가 및 삭제하는 함수
	virtual void AddGameObject(const shared_ptr<GameObject>& pGameObject) = 0;
	virtual void DeleteGameObject(const shared_ptr<GameObject>& pGameObject) = 0;

};

