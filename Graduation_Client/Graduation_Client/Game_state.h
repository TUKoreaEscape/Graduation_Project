#pragma once
#include "stdafx.h"

enum GAME_STATE {
	LOGIN = 0, //  로그인
	ROOM_SELECT, //방 선택
	WAITING_GAME, //대기실
	CUSTOMIZING, //커스터마이징
	READY_TO_GAME, //술래가 정해지기 전 게임 시작 전
	PLAYING_GAME, //게임중
	ENDING_GAME, //게임이 끝난 후
};

class GameState {

private:
	static GameState* GameStateInstance;
	GAME_STATE m_GameState; 
	float				m_totalLoading = 2.0f;
	float				m_Loading = 0.0f;
	int					m_LoadingCount = 0;
	GameState() {}
	GameState(const GameState& other);
	~GameState() {}

public:
	static GameState* GetInstance() {
		if (GameStateInstance == NULL) {
			GameStateInstance = new GameState;
			GameStateInstance->m_GameState = PLAYING_GAME;
		}
		return GameStateInstance;
	}
	void ChangeNextState();
	void ChangePrevState();
	void ChangeSameLevelState();
	bool IsLoading() { return m_totalLoading > m_Loading; }; //로딩중이면 true
	void UpdateLoading(float time) { m_Loading += time, m_LoadingCount = (m_LoadingCount + 1) % 4; };
	void SetLoading(float time) { m_totalLoading = time, m_Loading = 0.0f, m_LoadingCount = 0; };
	int LoadingCount() { return m_LoadingCount; };
	GAME_STATE GetGameState() { return m_GameState; };
};