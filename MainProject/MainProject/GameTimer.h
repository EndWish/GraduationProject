#pragma once

class GameTimer {

private:
	float m_timeElapsed;	// 경과시간

	long long m_frequencyPerSec;	// 초당 진동수 (Hz)
	double m_period;		// 주기 (진동당 경과시간)

	long long m_currentPerformanceCounter;	// 처음부터 Tick을 계산하는 순간까지의 진동수
	long long m_lastPerformanceCounter;	// 지난 Tick의 까지의 진동수
	long long m_pausedPerformanceCounter;
	
	queue<float> m_frameTimes;	// m_frameTimes 의 합이 1초가 되도록 유지 함으로써 m_frameTimes의 개수가 FPS가 된다.
	float m_sumFrameTimes;

	bool m_paused;

public:
	GameTimer();
	~GameTimer();

	float GetTimeElapsed() const;
	int GetFPS() const;
	bool IsPaused() const;

	void Tick(float lockFPS = 0.0f);	// 한틱 진행한다.	lockFPS를 통해 FPS를 제한할 수 있다.
	void Pause();	// 일시정지
	void Resume();	// 재개
	void Reset();	// 리셋(수행되는 시점부터 다시 시간을 재며, 일시정지가 풀린다.), FPS정보는 계속 가지고 간다.
};

