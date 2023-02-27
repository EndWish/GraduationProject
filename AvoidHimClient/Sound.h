#pragma once
// https://202psj.tistory.com/1554

struct WavHeaderFormat {
////////// RIFF
	char chunkId[4];
	unsigned long chunkSize;
	char format[4];

////////// FMT
	char subChunkId[4];
	unsigned long subChunkSize;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned long sampleRate;
	unsigned long bytesPerSecond;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
};

struct WaveDataFormat {
	////////// DATA
	char dataChunkId[4];
	unsigned long dataSize;
};

class Sound {
private:
	UINT sampleRate;
	UINT bitPerSample;
	// 사운드 데이터를 저장할 버퍼
	ComPtr<IDirectSoundBuffer8> pSecondaryBuffer;
	ComPtr<IDirectSound3DBuffer8> pSecondary3DBuffer;

public:
	Sound();
	~Sound();
	void Play(bool _loop = false);
	void SetSpeed(float _speed);
	void Stop();
	void Init(const ComPtr<IDirectSound8>& _pDirectSound, string _name);

	DWORD GetStatus() const;
	void SetPosition(const XMFLOAT3& _pos);
};

class SoundManager
{
private:
	UINT sampleRate;
	UINT bitPerSample;
	unordered_map<string, shared_ptr<Sound>> pSounds;
	ComPtr<IDirectSound8> pDirectSound;
	ComPtr<IDirectSoundBuffer> pPrimaryBuffer;
	ComPtr<IDirectSound3DListener8> pListener;


public:

	SoundManager();
	~SoundManager();
	void Init(HWND _hwnd);
	shared_ptr<Sound> LoadFile(string _name);
	void Play(string _name, bool _loop = false);
	void UpdateListener(const XMFLOAT3& _pos, const XMFLOAT3& _look, const XMFLOAT3& _up);
};
