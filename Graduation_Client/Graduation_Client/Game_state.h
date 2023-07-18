#pragma once
#include "stdafx.h"

enum GAME_STATE {
	LOGIN = 0, //  �α���
	ROOM_SELECT, //�� ����
	WAITING_GAME, //����
	CUSTOMIZING, //Ŀ���͸���¡
	READY_TO_GAME, //������ �������� �� ���� ���� ��
	PLAYING_GAME, //������
	ENDING_GAME, //������ ���� ��
	INTERACTION_POWER, // ������ġ ����
	GAME_LOADING,
};

class GameState {

private:
	static GameState* GameStateInstance;
	GAME_STATE m_GameState;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point taggerTime;
	long long taggerprevTime = 0;
	short taggercountdown = 60;
	bool mic_state = false; // ����ũ ��
	bool chat_state = false; // ä��

	long long prevTime = 0;
	bool initLight = false;
	float				m_totalLoading = 2.0f;
	float				m_Loading = 0.0f;
	int					m_LoadingCount = 0;
	bool				m_MinimapOn = false;
	GameState() {}
	GameState(const GameState& other);
	~GameState() {}

public:
	static GameState* GetInstance() {
		if (GameStateInstance == NULL) {
			GameStateInstance = new GameState;
			//GameStateInstance->m_GameState = LOGIN;
			GameStateInstance->m_GameState = GAME_LOADING;
		}
		return GameStateInstance;
	}
	void ChangeNextState();
	void ChangePrevState();
	void ChangeSameLevelState();
	bool IsLoading() { return m_totalLoading > m_Loading; }; //�ε����̸� true
	void UpdateLoading(float time) { m_Loading += time, m_LoadingCount = (m_LoadingCount + 1) % 4; };
	void SetLoading(float time) { m_totalLoading = time, m_Loading = 0.0f, m_LoadingCount = 0; };
	int LoadingCount() { return m_LoadingCount; };
	GAME_STATE GetGameState() { return m_GameState; };
	bool GetTick();
	bool GetInitLight() { return initLight; };
	void SetInitLight() { initLight = !initLight; };
	int GetTaggerTime();
	void ChangeMicState() { mic_state = !mic_state; };
	bool GetMicState() { return mic_state; };
	void ChangeChatState() { chat_state = !chat_state; };
	bool GetChatState() { return chat_state; };
	void ChangeMinimapState() { m_MinimapOn = !m_MinimapOn; };
	bool GetMinimapState() { return m_MinimapOn; };
};