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
	GameState() {}
	GameState(const GameState& other);
	~GameState() {}

public:
	static GameState* GetInstance() {
		if (GameStateInstance == NULL) {
			GameStateInstance = new GameState;
			GameStateInstance->m_GameState = LOGIN;
		}
		return GameStateInstance;
	}
	void ChangeNextState();
	void ChangePrevState();
	void ChangeSameLevelState();
	GAME_STATE GetGameState() { return m_GameState; };
};