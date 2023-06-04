#pragma once
#include "Game_state.h"

GameState* GameState::GameStateInstance = nullptr;

void GameState::ChangeState()
{
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			m_GameState = ROOM_SELECT;
			break;
		case ROOM_SELECT:
			m_GameState = READY_TO_GAME;
			break;
		case READY_TO_GAME:
			m_GameState = PLAYING_GAME;
			break;
		case PLAYING_GAME:
			m_GameState = READY_TO_GAME;
			break;
	}
	std::cout << m_GameState << std::endl;
}
