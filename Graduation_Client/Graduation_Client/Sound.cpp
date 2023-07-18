#include "stdafx.h"
#include "sound.h"
Sound* Sound::SoundInstance = nullptr;

Sound::Sound() : m_pSystem(nullptr), m_pChannel(nullptr), m_pSound(nullptr), m_uiVersion(0), m_nSounds(0)
{
}

void Sound::StartFMOD()
{
	FMOD::System_Create(&m_pSystem);
	m_pSystem->getVersion(&m_uiVersion);
	m_pSystem->init(MAX_CHANNEL, FMOD_INIT_NORMAL, nullptr);
}

int Sound::CreateEffectSound(char* file, float volume)
{
	if (m_pSystem) {
		if (m_pSystem->createSound(file, FMOD_LOOP_OFF, 0, &m_pSound) == FMOD_OK) {
			m_vSounds.push_back(m_pSound);
			m_nSounds++;
			return m_nSounds - 1;
		}
	}
	return -1;
}

int Sound::CreateBGSound(char* file, float volume)
{
	if (m_pSystem) {
		if (m_pSystem->createSound(file, FMOD_LOOP_NORMAL, 0, &m_pSound) == FMOD_OK) {
			m_vSounds.push_back(m_pSound);
			m_nSounds++;
			return m_nSounds - 1;
		}
	}
	return -1;
}

void Sound::Play(int index)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	m_pChannel->setVolume(0.5f);
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_pChannel);
}

void Sound::Stop(int index)
{

	if (index == -1) return;
	if (index > m_nSounds) return;
	m_pChannel->stop();
}

