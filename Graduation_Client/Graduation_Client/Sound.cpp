#include "stdafx.h"
#include "sound.h"
Sound* Sound::SoundInstance = nullptr;

Sound::Sound() : m_pSystem(nullptr), m_pChannel(nullptr), m_pSound(nullptr), m_uiVersion(0), m_nSounds(0)
{
}

void Sound::StartFMOD()
{
	FMOD_RESULT fr;
	FMOD::System_Create(&m_pSystem);
	m_pSystem->getVersion(&m_uiVersion);
	m_pSystem->init(MAX_CHANNEL, FMOD_INIT_NORMAL, nullptr);
	//m_pSystem->createReverb3D(&m_pReverb);
	//m_pReverb->setProperties(&prop);

	/*m_fvPos = { -10.0f, 0.0f, 0.0f };
	m_fMinDist = 10.0f;
	m_fMaxDist = 20.0f;
	m_pReverb->set3DAttributes(&m_fvPos, m_fMinDist, m_fMaxDist);*/

	m_fvListenerPos = { 0.0f, 0.0f, -1.0f };
	m_pSystem->set3DListenerAttributes(0, &m_fvListenerPos, 0, 0, 0);
	for (auto& chan : m_arrOtherPlayerChannel) {
		for (auto& ch : chan)
			ch = nullptr;
	}
	for (auto& chan : m_arrPlayerChannel) {
		chan = nullptr;
	}
	for (auto& cha : m_arrPlayerChannel) {
		cha = nullptr;
	}
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

int Sound::CreateObjectSound(char* file, const XMFLOAT3& position)
{
	m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_LINEARROLLOFF, 0, &m_pSound);
	m_pSound->set3DMinMaxDistance(10.0f, 50.0f);
	m_vObjectSounds.push_back(m_pSound);
	m_vObjectChannels.push_back(nullptr);
	FMOD_VECTOR v = { position.x, position.y, position.z };
	m_vObjectPosition.push_back(v);
	m_nObjects++;
	return m_nObjects - 1;
}

int Sound::CreateObjectSound(char* file, float x, float y, float z, float min, float max, bool loop)
{
	if (true == loop) {
		m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_NORMAL | FMOD_3D_LINEARROLLOFF, 0, &m_pSound);
	}
	else {
		m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_LINEARROLLOFF, 0, &m_pSound);
	}
	m_pSound->set3DMinMaxDistance(min, max);
	m_vObjectSounds.push_back(m_pSound);
	m_vObjectChannels.push_back(nullptr);
	FMOD_VECTOR v = { x,y,z };
	m_vObjectPosition.push_back(v);
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

int Sound::CreatePlayerEffectSound(char* file, float min, float max)
{
	if (m_pSystem) {
		if (m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_LINEARROLLOFF, 0, &m_pSound) == FMOD_OK) {
			m_pSound->set3DMinMaxDistance(min, max);
			m_vSounds.push_back(m_pSound);
			m_nSounds++;
			return m_nSounds - 1;
		}
	}
	return -1;
}

void Sound::Play(int index, float volume, int otherPlayer, int i)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	bool isPlaying = false;
	if (otherPlayer != -1) {
		m_arrOtherPlayerChannel[otherPlayer][i]->isPlaying(&isPlaying);
		if (isPlaying) return;
		m_pSystem->playSound(m_vSounds[index], 0, true, &m_arrOtherPlayerChannel[otherPlayer][i]);
		m_arrOtherPlayerChannel[otherPlayer][i]->setVolume(volume);
		m_arrOtherPlayerChannel[otherPlayer][i]->set3DAttributes(&m_arrOtherPlayerPos[otherPlayer], 0);
		m_arrOtherPlayerChannel[otherPlayer][i]->setPaused(false);
		return;
	}
	m_arrPlayerChannel[i]->isPlaying(&isPlaying);
	if (isPlaying) return;
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_arrPlayerChannel[i]);
	m_pChannel->setVolume(volume);
}

void Sound::PlayBG(int index)
{
	if (index == -1) return;
	if (index > m_nSounds) return;
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_pBGChannel);
	m_pBGChannel->setVolume(0.05f);
	//m_pBGChannel->setMute(true);
}

void Sound::PlayObjectSound(int index, float volume)
{
	if (index == -1) return;
	m_pSystem->playSound(m_vObjectSounds[index], nullptr, true, &m_vObjectChannels[index]);
	m_vObjectChannels[index]->setVolume(volume);
	m_vObjectChannels[index]->set3DAttributes(&m_vObjectPosition[index], 0);
	m_vObjectChannels[index]->setPaused(false);
}

void Sound::PlayEffectSound(int index, float volume)
{
	if (index == -1) return;
	m_pSystem->playSound(m_vSounds[index], nullptr, false, &m_pEffectChannel);
}

void Sound::PlayPlayerEffectSound(int index, int playerIndex, float volume)
{
	if (index == -1) return;
	if (playerIndex < 5) {
		m_pSystem->playSound(m_vSounds[index], nullptr, true, &m_arrPlayersEffectChannel[playerIndex]);
		m_arrPlayersEffectChannel[playerIndex]->setVolume(volume);
		m_arrPlayersEffectChannel[playerIndex]->set3DAttributes(&m_vObjectPosition[playerIndex], 0);
		m_arrPlayersEffectChannel[playerIndex]->setPaused(false);
	}
	else if (playerIndex == 5) {
		m_pSystem->playSound(m_vSounds[index], nullptr, true, &m_arrPlayersEffectChannel[playerIndex]);
		m_arrPlayersEffectChannel[playerIndex]->setVolume(volume);
		m_arrPlayersEffectChannel[playerIndex]->set3DAttributes(&m_fvListenerPos, 0);
		m_arrPlayersEffectChannel[playerIndex]->setPaused(false);
	}
	else {
		return;
	}
}

void Sound::Stop(int index, int OtherPlayer)
{

}

void Sound::StopBG(int index)
{
	if (index == -1) return;
	m_pBGChannel->stop();
}

void Sound::StopObjectSound(int index)
{
	if (index == -1) return;
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

int Sound::CreatePlayersSounds(char* file, int index, float min, float max)
{
	m_pSystem->createSound(file, FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_LINEARROLLOFF, 0, &m_pSound);
	m_pSound->set3DMinMaxDistance(min, max);
	m_vSounds.push_back(m_pSound);
	m_nSounds++;
	return m_nSounds - 1;
}

void Sound::SetOtherPlayersPos(int index, const XMFLOAT3& pos)
{
	m_arrOtherPlayerPos[index] = {pos.x, pos.y, pos.z};
}

void Sound::SetObjectPos(int index, float x, float y, float z)
{
	m_vObjectPosition[index] = { x,y,z };
}

