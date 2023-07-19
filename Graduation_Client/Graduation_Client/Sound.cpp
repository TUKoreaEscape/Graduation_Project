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
	m_pSystem->createReverb3D(&m_pReverb);
	m_pReverb->setProperties(&prop);

	m_fvPos = { -10.0f, 0.0f, 0.0f };
	m_fMinDist = 10.0f;
	m_fMaxDist = 20.0f;
	m_pReverb->set3DAttributes(&m_fvPos, m_fMinDist, m_fMaxDist);

	m_fvListenerPos = { 0.0f, 0.0f, -1.0f };
	m_pSystem->set3DListenerAttributes(0, &m_fvListenerPos, 0, 0, 0);
}

int Sound::CreateEffectSound(char* file, float volume)
{
	if (m_pSystem) {
		if (m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF, 0, &m_pSound) == FMOD_OK) {
			m_vSounds.push_back(m_pSound);
			m_nSounds++;
			return m_nSounds - 1;
		}
	}
	return -1;
}

int Sound::CreateObjectSound(char* file)
{
	m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_LINEARROLLOFF, 0, &m_pSound);
	m_pSound->set3DMinMaxDistance(10.0f, 50.0f);
	m_vObjectSounds.push_back(m_pSound);
	m_vObjectChannels.push_back(nullptr);
	m_nObjects++;
	return m_nObjects - 1;
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

void Sound::Play(int index, float volume, int otherPlayer)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	if (otherPlayer != -1) {
		m_pSystem->playSound(m_vSounds[index], 0, true, &m_arrOtehrPlayerChannel[otherPlayer]);
		m_arrOtehrPlayerChannel[otherPlayer]->setVolume(volume);
		return;
	}
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_pChannel);
	m_pChannel->setVolume(volume);
}

void Sound::PlayBG(int index)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_pBGChannel);
	m_pBGChannel->setVolume(0.05f);
}

void Sound::PlayObject(int index, float volume, float x, float y, float z)
{
	m_pSystem->playSound(m_vObjectSounds[index], nullptr, true, &m_vObjectChannels[index]);
	m_vObjectChannels[index]->setVolume(volume);
	SetObjectPos(index, x, y, z);
	m_vObjectChannels[index]->setPaused(false);
}

void Sound::Stop(int index, int OtherPlayer)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	if (OtherPlayer != -1) {
		m_arrOtehrPlayerChannel[OtherPlayer]->stop();
	}
	else
		m_pChannel->stop();
}

void Sound::StopBG(int index)
{
	m_pBGChannel->stop();
}

void Sound::StopObject(int index)
{
	m_vObjectChannels[index]->stop();
}

void Sound::Update(float fElapsedTime)
{
	m_pSystem->update();
}

void Sound::SetListenerPos(const XMFLOAT3& listenerPos, const XMFLOAT3& listenerLook, const XMFLOAT3& listenerUp)
{
	m_fvListenerPos = { listenerPos.x, listenerPos.y, listenerPos.z };
	m_fvListenerLook = { listenerLook.x, listenerLook.y, listenerLook.z };
	m_fvListenerUp = { listenerUp.x, listenerUp.y, listenerUp.z };
	m_pSystem->set3DListenerAttributes(0, &m_fvListenerPos, 0, &m_fvListenerLook, &m_fvListenerUp);
}

int Sound::CreateOtherPlayersSounds(int index)
{
	m_pSystem->createSound("Sound/Footstep01.wav", FMOD_3D | FMOD_LOOP_OFF, 0, &m_pSound);
	m_vSounds.push_back(m_pSound);
	m_nSounds++;
	return m_nSounds - 1;
}

void Sound::SetOtherPlayersPos(int index, const XMFLOAT3& pos)
{
	FMOD_VECTOR fmodPos = { pos.x, pos.y, pos.z };
	m_arrOtehrPlayerChannel[index]->set3DAttributes(&fmodPos, 0);
}

void Sound::SetObjectPos(int index, float x, float y, float z)
{
	FMOD_VECTOR pos = { x, y, z };
	m_vObjectChannels[index]->set3DAttributes(&pos, 0);
}

