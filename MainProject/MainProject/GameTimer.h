#pragma once

class GameTimer {

private:
	float m_timeElapsed;	// ����ð�

	long long m_frequencyPerSec;	// �ʴ� ������ (Hz)
	double m_period;		// �ֱ� (������ ����ð�)

	long long m_currentPerformanceCounter;	// ó������ Tick�� ����ϴ� ���������� ������
	long long m_lastPerformanceCounter;	// ���� Tick�� ������ ������
	long long m_pausedPerformanceCounter;
	
	queue<float> m_frameTimes;	// m_frameTimes �� ���� 1�ʰ� �ǵ��� ���� �����ν� m_frameTimes�� ������ FPS�� �ȴ�.
	float m_sumFrameTimes;

	bool m_paused;

public:
	GameTimer();
	~GameTimer();

	float GetTimeElapsed() const;
	int GetFPS() const;
	bool IsPaused() const;

	void Tick(float lockFPS = 0.0f);	// ��ƽ �����Ѵ�.	lockFPS�� ���� FPS�� ������ �� �ִ�.
	void Pause();	// �Ͻ�����
	void Resume();	// �簳
	void Reset();	// ����(����Ǵ� �������� �ٽ� �ð��� ���, �Ͻ������� Ǯ����.), FPS������ ��� ������ ����.
};

