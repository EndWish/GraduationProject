#pragma once
class Sound
{
private:

public:
	ComPtr<IDirectSound8> pDirectSound;
	ComPtr<IDirectSoundBuffer> pPrimaryBuffer;
	ComPtr<IDirectSoundBuffer8> pSecondaryBuffer;

};

