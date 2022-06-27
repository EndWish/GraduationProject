#include "stdafx.h"
#include "GameTimer.h"

// 생성자 및 소멸자
GameTimer::GameTimer()
{
	m_timeElapsed = 0;

	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequencyPerSec);	// 초당 진동 수를 반환
	m_period = 1.0 / (double)m_frequencyPerSec;	// 1회 진동시 걸리는 시간.

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_lastPerformanceCounter);		// 처음부터 지금까지의 진동수 반환
	m_currentPerformanceCounter = m_lastPerformanceCounter;
	m_pausedPerformanceCounter = 0;

	m_sumFrameTimes = 0;

	m_paused = false;
}
GameTimer::~GameTimer() 
{

}

// get set 함수
float GameTimer::GetTimeElapsed() 
{
#ifdef DEBUG
	if (m_paused)
		cout << "GameTimer::GetTimeElapsed() : 일시정지 상태에서 함수를 호출했습니다.\n";
#endif // DEBUG

	return m_timeElapsed;
}
int GameTimer::GetFPS() 
{
#ifdef DEBUG
	if (m_paused)
		cout << "GameTimer::GetFPS() : 일시정지 상태에서 함수를 호출했습니다.\n";
#endif // DEBUG

	return m_frameTimes.size() + 1;
}
bool GameTimer::IsPaused() 
{
	return m_paused;
}

// 한 틱 진행시킨다.
void GameTimer::Tick(float lockFPS) 
{
#ifdef DEBUG
	if (m_paused) {
		cout << "GameTimer::Tick() : 일시정지 상태에서 Tick()함수를 호출했습니다.\n";
		m_timeElapsed = 0.0f;
		return;
}
#endif // DEBUG

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);	// 현재 까지의 진동수를 가져온다.
	m_timeElapsed = float((m_currentPerformanceCounter - m_lastPerformanceCounter) * m_period);	// 한 틱 동안의 진동수(진동수/1틱) * 진동당 경과시간(경과시간/1진동) = 한틱 동안의 경과시간(경과시간/1틱)

	// lockFPS보다 더 성능이 나온다면, lockFPS 가 될때 까지 대기한다.
	if (lockFPS > 0.0f) {
		while(m_timeElapsed < (1.0f / lockFPS)) {
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);
			m_timeElapsed = float((m_currentPerformanceCounter - m_lastPerformanceCounter) * m_period);
		}
	}

	m_lastPerformanceCounter = m_currentPerformanceCounter;	// 다음 틱에서는 "지금의 진동수"가 "이전의 진동수"가 된다.

	// FPS계산하기 : FPS = 1초(m_sumFrameTimes) 동안 지나간 프레임수(m_frameTimes.size())
	m_frameTimes.push(m_timeElapsed);
	m_sumFrameTimes += m_timeElapsed;
	while (m_sumFrameTimes > 1.0f) {	// m_sumFrameTimes가 1초가 될때 까지 pop() 한다.
		m_sumFrameTimes -= m_frameTimes.front();
		m_frameTimes.pop();
	}
}
// 일시정지 및 재개
void GameTimer::Pause()
{
#ifdef DEBUG
	if (m_paused) {
		cout << "GameTimer::Pause() : 이미 일지정지를 한 상태인데 한번더 호출했습니다.(일시정지 하기 전까지 지나간 시간이 바뀌어 오류가 생긴다.)\n";
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
		cout << "GameTimer::Resume() : 일시 정지된 적이 없습니다.\n";
#endif // DEBUG

	long long lastToPausedPerformanceCounter = m_pausedPerformanceCounter - m_lastPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);	// 현재까지의 진동수(==현재시각)를 가져온다.
	m_lastPerformanceCounter -= m_currentPerformanceCounter - lastToPausedPerformanceCounter;	// 이전까지의 진동수(이전시각)를 일시정지 하기전 지나간 시간만큼 현재 진동수(현재시각)에서 뺀 값으로 변환
	m_paused = false;
}
// 리셋 (수행되는 시점부터 다시 시간을 재며, 일시정지가 풀린다.), FPS정보는 계속 가지고 간다.
void GameTimer::Reset() 
{
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentPerformanceCounter);
	m_lastPerformanceCounter = m_currentPerformanceCounter;
	m_paused = false;
}