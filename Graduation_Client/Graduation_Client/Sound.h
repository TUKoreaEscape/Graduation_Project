#pragma once

constexpr int MAX_CHANNEL = 32;

class Sound
{
private:
	static Sound* SoundInstance;
	Sound();
public:
	static Sound* GetInstance() {
		if (SoundInstance == nullptr) {
			SoundInstance = new Sound;
		}
		return SoundInstance;
	}
	~Sound() { m_pSystem->release(); }

	void StartFMOD();
	int CreateEffectSound(char* file, float volume);
	int CreateBGSound(char* file, float volume);

	void Play(int index);
	void Stop(int index);
private:
	FMOD::System* m_pSystem;
	FMOD::Sound* m_pSound;
	FMOD::Channel* m_pChannel;
	unsigned int m_uiVersion;

	int m_nSounds;
	std::vector<FMOD::Sound*> m_vSounds;
};
