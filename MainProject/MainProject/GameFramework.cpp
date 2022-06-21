#include "stdafx.h"
#include "GameFramework.h"

// 기본 생성자
GameFramework::GameFramework() 
{
	// 윈도우
	m_hInstance = NULL;
	m_hWnd = NULL;

	// 클라이언트 해상도
	m_windowClientWidth = FRAME_BUFFER_WIDTH;
	m_windowClientHeight = FRAME_BUFFER_HEIGHT;

	// 팩토리, 스왑체인, 디바이스
	m_pDxgiFactory = NULL;
	m_pDxgiSwapChain = NULL;
	m_pD3dDevice = NULL;

	// MSAA 다중 샘플링
	m_msaa4xEnable = false;
	m_msaa4xQualityLevels = 0;

	// 현재 스왑체인의 후면 버퍼의 인덱스
	m_swapChainBufferCurrentIndex = 0;

	// 렌더 타겟 초기화
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_ppD3dRenderTargetBuffers[i] = NULL;
	m_pd3dRtvDescriptorHeap = NULL;
	m_rtvDescriptorIncrementSize = 0;

	// 깊이-스텐실 초기화
	m_pD3dDepthStencilBuffer = NULL;
	m_pD3dDsvDescriptorHeap = NULL;
	m_dsvDescriptorIncrementSize = 0;

	// 명령 큐, 할당자, 리스트 초기화
	m_pD3dCommandQueue = NULL;
	m_pD3dCommandAllocator = NULL;
	m_pD3dCommandList = NULL;

	//펜스 초기화
	m_pD3dFence = NULL;
	m_fenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_fenceValues[i] = NULL;

}
// 소멸자
GameFramework::~GameFramework() 
{

}


