#include "stdafx.h"
#include "GameTimer.h"

// ������ �� �Ҹ���
GameTimer::GameTimer()
{
	m_timeElapsed = 0;

	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequencyPerSec);	// �ʴ� ���� ���� ��ȯ
	m_period = 1.0 / (double)m_frequencyPerSec;	// 1ȸ ������ �ɸ��� �ð�.

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_lastPerformanceCounter);		// ó������ ���ݱ����� ������ ��ȯ
	m_currentPerformanceCounter = m_lastPerformanceCounter;
	m_pausedPerformanceCounter = 0;

	m_sumFrameTimes = 0;

	m_paused = false;
}
GameTimer::~GameTimer() 
{

}

// get set �Լ�
float GameTimer::GetTimeElapsed() 
{
#ifdef DEBUG
	if (m_paused)
		cout << "GameTimer::GetTimeElapsed() : �Ͻ����� ���¿��� �Լ��� ȣ���߽��ϴ�.\n";
#endif // DEBUG

	return m_timeElapsed;
}
int GameTimer::GetFPS() 
{
#ifdef DEBUG
	if (m_paused)
		cout << "GameTimer::GetFPS() : �Ͻ����� ���¿��� �Լ��� ȣ���߽��ϴ�.\n";
#endif // DEBUG

	return m_frameTimes.size() + 1;
}
bool GameTimer::IsPaused() 
{
	return m_paused;
}

// �� ƽ �����Ų��.
void GameTimer::Tick(float lockFPS) 
{
#ifdef DEBUG
	if (m_paused) {
		cout << "GameTimer::Tick() : �Ͻ����� ���¿��� Tick()�Լ��� ȣ���߽��ϴ�.\n";
		m_timeElapsed = 0.0f;
		return;
}
#endif // DEBUG

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);	// ���� ������ �������� �����´�.
	m_timeElapsed = float((m_currentPerformanceCounter - m_lastPerformanceCounter) * m_period);	// �� ƽ ������ ������(������/1ƽ) * ������ ����ð�(����ð�/1����) = ��ƽ ������ ����ð�(����ð�/1ƽ)

	// lockFPS���� �� ������ ���´ٸ�, lockFPS �� �ɶ� ���� ����Ѵ�.
	if (lockFPS > 0.0f) {
		while(m_timeElapsed < (1.0f / lockFPS)) {
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);
			m_timeElapsed = float((m_currentPerformanceCounter - m_lastPerformanceCounter) * m_period);
		}
	}

	m_lastPerformanceCounter = m_currentPerformanceCounter;	// ���� ƽ������ "������ ������"�� "������ ������"�� �ȴ�.

	// FPS����ϱ� : FPS = 1��(m_sumFrameTimes) ���� ������ �����Ӽ�(m_frameTimes.size())
	m_frameTimes.push(m_timeElapsed);
	m_sumFrameTimes += m_timeElapsed;
	while (m_sumFrameTimes > 1.0f) {	// m_sumFrameTimes�� 1�ʰ� �ɶ� ���� pop() �Ѵ�.
		m_sumFrameTimes -= m_frameTimes.front();
		m_frameTimes.pop();
	}
}
// �Ͻ����� �� �簳
void GameTimer::Pause()
{
#ifdef DEBUG
	if (m_paused) {
		cout << "GameTimer::Pause() : �̹� ���������� �� �����ε� �ѹ��� ȣ���߽��ϴ�.(�Ͻ����� �ϱ� ������ ������ �ð��� �ٲ�� ������ �����.)\n";
		return;
	}
		
#endif // DEBUG

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_pausedPerformanceCounter);
	m_paused = true;
}
void GameTimer::Resume() 
{
#ifdef DEBUG
	if (!m_paused)
		cout << "GameTimer::Resume() : �Ͻ� ������ ���� �����ϴ�.\n";
#endif // DEBUG

	long long lastToPausedPerformanceCounter = m_pausedPerformanceCounter - m_lastPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);	// ��������� ������(==����ð�)�� �����´�.
	m_lastPerformanceCounter -= m_currentPerformanceCounter - lastToPausedPerformanceCounter;	// ���������� ������(�����ð�)�� �Ͻ����� �ϱ��� ������ �ð���ŭ ���� ������(����ð�)���� �� ������ ��ȯ
	m_paused = false;
}
// ���� (����Ǵ� �������� �ٽ� �ð��� ���, �Ͻ������� Ǯ����.), FPS������ ��� ������ ����.
void GameTimer::Reset() 
{
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);
	m_lastPerformanceCounter = m_currentPerformanceCounter;
	m_paused = false;
}