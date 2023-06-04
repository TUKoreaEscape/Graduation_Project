#pragma once
#include "stdafx.h"

enum GAME_STATE {
	LOGIN = 0,
	ROOM_SELECT,
	READY_TO_GAME,
	PLAYING_GAME,
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
	void ChangeState();
	GAME_STATE GetGameState() { return m_GameState; };
};