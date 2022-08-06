#pragma once
class Timer	{
private:
	double timeElapsed;			// ����ð�

	long long frequencyPerSec;	// CPU ������(Hz����)
	double period;				// �ֱ�(������ ����ð�)

	long long totalFrequency;	// ���� ������
	long long lastFrequency;	// ���� �����ӿ����� ���� ������
	long long pauseFrequency;

	queue<double> frameTimes;	// ť �� ������ Ÿ�ӵ��� ���� 1�ʰ� �ǵ��� �Ͽ� FPS ���
	double sumFrameTime;		// ť �� ������ Ÿ�ӵ��� ���� ����

	bool paused;				// ���� ����

public:
	Timer();
	~Timer();

	double GetTimeElapsed() const;
	int GetFPS() const;
	bool IsPaused() const;

	void Tick(double lockFPS = 0.0f);	// �� ������ ����
	void Pause();
	void Resume();
	void Reset();
};

