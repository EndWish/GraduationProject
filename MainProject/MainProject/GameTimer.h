#pragma once

class GameTimer {

private:
	float m_timeElapsed;	// ����ð�

	long long m_frequencyPerSec;	// �ʴ� ������ (Hz)
	double m_period;		// �ֱ� (������ ����ð�)

	long long m_currentPerformanceCounter;	// ó������ Tick�� ����ϴ� ���������� ������
	long long m_lastPerformanceCounter;	// ���� Tick�� ������ ������
	long long m_pausedPerformanceCounter;
	
	queue<float> m_frameTimes;
	float m_sumFrameTimes;
	int m_framePerSec;	// FPS = 1��(m_sumFrameTimes) ���� ������ �����Ӽ�(m_frameTimes.size())

	bool m_paused;

public:
	GameTimer();
	~GameTimer();

	float GetTimeElapsed();
	int GetFPS();
	bool IsPaused();

	void Tick(float lockFPS = 0.0f);	// ��ƽ �����Ѵ�.	lockFPS�� ���� FPS�� ������ �� �ִ�.
	void Pause();	// �Ͻ�����
	void Resume();	// �簳
	void Reset();	// ����(����Ǵ� �������� �ٽ� �ð��� ���, �Ͻ������� Ǯ����.), FPS������ ��� ������ ����.
};

