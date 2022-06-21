#include "stdafx.h"
#include "GameFramework.h"

// �⺻ ������
GameFramework::GameFramework() 
{
	// ������
	m_hInstance = NULL;
	m_hWnd = NULL;

	// Ŭ���̾�Ʈ �ػ�
	m_windowClientWidth = FRAME_BUFFER_WIDTH;
	m_windowClientHeight = FRAME_BUFFER_HEIGHT;

	// ���丮, ����ü��, ����̽�
	m_pDxgiFactory = NULL;
	m_pDxgiSwapChain = NULL;
	m_pD3dDevice = NULL;

	// MSAA ���� ���ø�
	m_msaa4xEnable = false;
	m_msaa4xQualityLevels = 0;

	// ���� ����ü���� �ĸ� ������ �ε���
	m_swapChainBufferCurrentIndex = 0;

	// ���� Ÿ�� �ʱ�ȭ
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_ppD3dRenderTargetBuffers[i] = NULL;
	m_pd3dRtvDescriptorHeap = NULL;
	m_rtvDescriptorIncrementSize = 0;

	// ����-���ٽ� �ʱ�ȭ
	m_pD3dDepthStencilBuffer = NULL;
	m_pD3dDsvDescriptorHeap = NULL;
	m_dsvDescriptorIncrementSize = 0;

	// ��� ť, �Ҵ���, ����Ʈ �ʱ�ȭ
	m_pD3dCommandQueue = NULL;
	m_pD3dCommandAllocator = NULL;
	m_pD3dCommandList = NULL;

	//�潺 �ʱ�ȭ
	m_pD3dFence = NULL;
	m_fenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_fenceValues[i] = NULL;

}
// �Ҹ���
GameFramework::~GameFramework() 
{

}


