#pragma once
#include "Game_state.h"

GameState* GameState::GameStateInstance = nullptr;

void GameState::ChangeNextState()
{
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			m_GameState = ROOM_SELECT;
			break;
		case ROOM_SELECT:
			m_GameState = WAITING_GAME;
			break;
		case WAITING_GAME:
			m_GameState = READY_TO_GAME;
			break;
		case CUSTOMIZING:
			break;
		case READY_TO_GAME:
			m_GameState = PLAYING_GAME;
			break;
		case PLAYING_GAME:
			m_GameState = ENDING_GAME;
			break;
		case ENDING_GAME:
			m_GameState = WAITING_GAME;
			break;
	}
	std::cout << m_GameState << std::endl;
}

void GameState::ChangePrevState()
{
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			break;
		case ROOM_SELECT:
			m_GameState = LOGIN;
			break;
		case WAITING_GAME:
			m_GameState = ROOM_SELECT;
			break;
		case CUSTOMIZING:
			break;
		case READY_TO_GAME:
			m_GameState = WAITING_GAME;
			break;
		case PLAYING_GAME:
			m_GameState = READY_TO_GAME;
			break;
		case ENDING_GAME:
			m_GameState = PLAYING_GAME;
			break;
	}
	std::cout << m_GameState << std::endl;
}

void GameState::ChangeSameLevelState()
{
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			break;
		case ROOM_SELECT:
			break;
		case WAITING_GAME:
			m_GameState = CUSTOMIZING;
			break;
		case CUSTOMIZING:
			m_GameState = WAITING_GAME;
			break;
		case READY_TO_GAME:
			break;
		case PLAYING_GAME:
			break;
		case ENDING_GAME:
			break;
	}
	std::cout << m_GameState << std::endl;
}
