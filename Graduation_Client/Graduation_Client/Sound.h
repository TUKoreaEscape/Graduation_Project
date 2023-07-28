#pragma once
#include <array>

constexpr int MAX_CHANNEL = 64;

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
	int CreateObjectSound(char* file, const XMFLOAT3& position);
	int CreateObjectSound(char* file, float x, float y, float z, float min, float max, bool loop = false);
	int CreateBGSound(char* file, float volume);

	void Play(int index, float volume, int OtherPlayer = -1, int i = 0);
	void PlayBG(int index);
	void PlayObjectSound(int index, float volume);
	void PlayEffectSound(int index, float volume);
	void Stop(int index, int OtherPlayer = -1);
	void StopBG(int index);
	void StopObjectSound(int index);

	void Update(float fElapsedTime);
	void SetListenerPos(const XMFLOAT3& listenerPos, const XMFLOAT3& look, const XMFLOAT3& up);

	int CreatePlayersSounds(char* file, int index, float min, float max);
	void SetOtherPlayersPos(int index, const XMFLOAT3& pos);
	void SetObjectPos(int index, float x, float y, float z);
private:
	FMOD::System* m_pSystem;
	FMOD::Sound* m_pSound;
	FMOD::Channel* m_pChannel;
	FMOD::Reverb3D* m_pReverb;
	FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_ROOM;
	FMOD_VECTOR m_fvPos;
	float m_fMinDist;
	float m_fMaxDist;

	FMOD_VECTOR m_fvListenerPos;
	FMOD_VECTOR m_fvListenerLook;
	FMOD_VECTOR m_fvListenerUp;

	unsigned int m_uiVersion;

	int m_nSounds;
	std::vector<FMOD::Sound*> m_vSounds;
	std::vector<FMOD::Channel*> m_vChannels;
	std::array<std::array<FMOD::Channel*, 2>, 5> m_arrOtherPlayerChannel;
	std::array<FMOD_VECTOR, 5> m_arrOtherPlayerPos{};

	std::array<FMOD::Channel*, 2> m_arrPlayerChannel;
	
	int m_nObjects{};
	std::vector<FMOD::Sound*> m_vObjectSounds;
	std::vector<FMOD::Channel*> m_vObjectChannels;
	std::vector<FMOD_VECTOR> m_vObjectPosition;

	FMOD::Channel* m_pBGChannel;
	FMOD::Channel* m_pEffectChannel;
};
