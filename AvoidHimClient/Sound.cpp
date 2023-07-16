#include "stdafx.h"
#include "Sound.h"


Sound::Sound() {
	sampleRate = 44100; // 44100Hz
	bitPerSample = 16;	// 16bits	
}

Sound::~Sound() {

}

void Sound::Play(bool _loop) {

	if (!(GetStatus() & DSBSTATUS_LOOPING)) {
		DWORD flag = _loop ? DSBPLAY_LOOPING : 0;

		pSecondaryBuffer->SetCurrentPosition(0);

		pSecondaryBuffer->Play(0, 0, flag);
	}
}

void Sound::SetFreq(float _freq) {
	// 지정된 샘플레이트를 벗어나게 하지 말것
	pSecondaryBuffer->SetFrequency(DWORD(sampleRate * _freq));
}

void Sound::Stop() {
	if (GetStatus() & DSBSTATUS_LOOPING || GetStatus() & DSBSTATUS_PLAYING) {

		pSecondaryBuffer->Stop();
	}
}

void Sound::Init(const ComPtr<IDirectSound8>& _pDirectSound, string _name) {
	WavHeaderFormat format;
	UINT dataSize;
	ifstream in(_name, ios::binary);
	if (in) {
		in.read((char*)&format, sizeof(WavHeaderFormat));
	}
	else {
		cout << "file load failed : " << _name << "\n";
	}
	if (in.fail()) cout << "file read failed : " << _name << "\n";
	// Wav 헤더를 먼저 읽는다.
	if (strncmp(format.chunkId, "RIFF", 4) != 0) {
		cout << "chunk id error : " << _name << "\n";
	}
	if (strncmp(format.format, "WAVE", 4) != 0) {
		cout << "format error : " << _name << "\n";
	}
	if (strncmp(format.subChunkId, "fmt ", 4) != 0) {
		cout << "sub chunk id error : " << _name << "\n";
	}
	if (format.audioFormat != WAVE_FORMAT_PCM) {
		cout << "audio format error : " << _name << "\n";
	}
	if (format.sampleRate != sampleRate) {
		cout << "sampleRate error : " << _name << "\n";
	}
	if (format.bitsPerSample != bitPerSample) {
		cout << "bitPerSample error : " << _name << "\n";
	}
	string str;
	char dataPadding = 0;
	while (1) {
		in.read(&dataPadding, sizeof(char));
		if (dataPadding == 'd') {
			in.read(&dataPadding, sizeof(char));
			if (dataPadding == 'a') {
				in.read(&dataPadding, sizeof(char));
				if (dataPadding == 't') {
					in.read(&dataPadding, sizeof(char));
					if (dataPadding == 'a') {
						break;
					}
				}
			}
		}
	}
	in.read((char*)&dataSize, sizeof(UINT));
	// 실제 오디오 데이터를 읽는다.
	BYTE* pcmData = new BYTE[dataSize];
	in.read((char*)pcmData, dataSize);
	if (in.fail()) cout << "file read failed : " << _name << "\n";

	WAVEFORMATEX Waveformat{ 0 };
	Waveformat.wFormatTag = WAVE_FORMAT_PCM;
	Waveformat.nChannels = 1;	// 모노
	Waveformat.nSamplesPerSec = sampleRate;
	Waveformat.wBitsPerSample = bitPerSample;
	Waveformat.nBlockAlign = Waveformat.wBitsPerSample / 8 * Waveformat.nChannels;
	Waveformat.nAvgBytesPerSec = Waveformat.nSamplesPerSec * Waveformat.nBlockAlign;

	DSBUFFERDESC secondaryBufferDesc{ 0 };
	secondaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	secondaryBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D | DSBCAPS_GLOBALFOCUS;
	secondaryBufferDesc.dwBufferBytes = dataSize;
	secondaryBufferDesc.lpwfxFormat = &Waveformat;
	secondaryBufferDesc.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer* tempBuffer = nullptr;
	if (FAILED(_pDirectSound->CreateSoundBuffer(&secondaryBufferDesc, &tempBuffer, NULL))) {
		cout << "Error : secondaryBuffer CreateSoundBuffer() : " << _name << "\n";
	}
	// 실제 버퍼에 복사
	if (FAILED(tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&pSecondaryBuffer))) {
		cout << "Error : QueryInterface() : " << _name << "\n";
	}
	tempBuffer->Release();
	tempBuffer = nullptr;

	BYTE* bufferPtr = nullptr;
	DWORD bufferSize = 0;
	// Lock후 버퍼에 읽은내용을 복사
	HRESULT hr;

	hr = pSecondaryBuffer->Lock(0, dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if (FAILED(hr)) {
		cout << "Error : Lock() : " << _name << "\n";
	}
	cout << bufferSize << " , " << dataSize << "\n";
	memcpy(bufferPtr, pcmData, dataSize);

	hr = pSecondaryBuffer->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if (FAILED(hr)) {
		cout << "Error : Unlock() : " << _name << "\n";
	}
	hr = pSecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&pSecondary3DBuffer);
	if (FAILED(hr)) {
		cout << "Error : second->3D buffer QueryInterface()  : " << _name << "\n";
	}

	pSecondary3DBuffer->SetMinDistance(10.0f, DS3D_IMMEDIATE);

	delete[] pcmData;
	pcmData = 0;
}

DWORD Sound::GetStatus() const {
	DWORD result;
	pSecondaryBuffer->GetStatus(&result);
	return result;
}

void Sound::SetPosition(const XMFLOAT3& _pos) {
	pSecondary3DBuffer->SetPosition(_pos.x, _pos.y, _pos.z, DS3D_IMMEDIATE);
}



SoundManager::SoundManager() {
	sampleRate = 44100; // 44100Hz
	bitPerSample = 16;	// 16bits	
}

SoundManager::~SoundManager() {

}

void SoundManager::Init(HWND _hwnd) {

	if (FAILED(DirectSoundCreate8(NULL, &pDirectSound, NULL))) {
		cout << "Error : DirectSoundCreate8()\n";
	}
	if (FAILED(pDirectSound->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY))) {
	//if (FAILED(pDirectSound->SetCooperativeLevel(_hwnd, DSSCL_NORMAL))) {
		cout << "Error : SetCooperativeLevel()\n";
	}

	// 1차 버퍼를 초기화하기 위한 desc
	DSBUFFERDESC primaryBufferDesc{ 0 };
	primaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	primaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	if (FAILED(pDirectSound->CreateSoundBuffer(&primaryBufferDesc, &pPrimaryBuffer, NULL))) {
		cout << "Error : primaryBuffer CreateSoundBuffer()\n";
	}
	WAVEFORMATEX Waveformat{ 0 };
	Waveformat.cbSize = sizeof(WAVEFORMATEX);
	Waveformat.wFormatTag = WAVE_FORMAT_PCM;
	Waveformat.nChannels = 1;	// 모노
	Waveformat.nSamplesPerSec = sampleRate;
	Waveformat.wBitsPerSample = bitPerSample;
	Waveformat.nBlockAlign = Waveformat.wBitsPerSample / 8 * Waveformat.nChannels;
	Waveformat.nAvgBytesPerSec = Waveformat.nSamplesPerSec * Waveformat.nBlockAlign;
	if (FAILED(pPrimaryBuffer->SetFormat(&Waveformat))) {
		cout << "Error : SetFormat()\n";
	}
	HRESULT hr = 0;
	hr = pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener8, (void**)&pListener);
	if (FAILED(hr)) {
		cout << "Error : QueryInterface() "<< "\n";
	}
	/////////////////////////////////

	//pSounds["audio"] = LoadFile("audio");
	pSounds["horror"] = LoadFile("horror", 1.0f);
	pSounds["step"] = LoadFile("step", 1.0f);
	pSounds["noneSound"] = LoadFile("noneSound");
	pSounds["water"] = LoadFile("water");
	pSounds["monster"] = LoadFile("monster");
	pSounds["radar"] = LoadFile("radar");

	pSounds["healSound"] = LoadFile("healSound");
	pSounds["hittingSound"] = LoadFile("hittingSound");
	pSounds["keyboardSound"] = LoadFile("keyboardSound");
	pSounds["leverSound"] = LoadFile("leverSound");
	pSounds["lightSound"] = LoadFile("lightSound");
	pSounds["openDoorSound"] = LoadFile("openDoorSound");
	pSounds["swingSound01"] = LoadFile("swingSound01");
	pSounds["swingSound02"] = LoadFile("swingSound02");

}

shared_ptr<Sound> SoundManager::LoadFile(string _name, float _freq) {
	shared_ptr<Sound> pSound = make_shared<Sound>();
	pSound->Init(pDirectSound, "Sound/" + _name + ".wav");
	pSound->SetFreq(_freq);
	return pSound;
}

void SoundManager::Play(const string& _name, bool _loop) {
	pSounds[_name]->Play(_loop);
}

void SoundManager::Stop(const string& _name) {
	pSounds[_name]->Stop();
}

void SoundManager::SetPosition(const string& _name, const XMFLOAT3& _pos) {
	pSounds[_name]->SetPosition(_pos);
}

void SoundManager::UpdateListener(const XMFLOAT3& _pos, const XMFLOAT3& _look, const XMFLOAT3& _up) {
	pListener->SetPosition(_pos.x, _pos.y, _pos.z, DS3D_DEFERRED);
	pListener->SetOrientation(_look.x, _look.y, _look.z, _up.x, _up.y, _up.z, DS3D_DEFERRED);
	pListener->CommitDeferredSettings();
}


