#pragma once
#include "Game_state.h"
#include "Input.h"
#include "Camera.h"

GameState* GameState::GameStateInstance = nullptr;

void GameState::ChangeNextState()
{
	Player* player = Input::GetInstance()->m_pPlayer;
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			m_GameState = ROOM_SELECT;
			break;
		case ROOM_SELECT:
			player->ChangeCamera(ROOM_SELECT, WAITING_GAME);
			m_GameState = WAITING_GAME;
			break;
		case WAITING_GAME:
			player->ChangeCamera(WAITING_GAME, READY_TO_GAME);
			m_GameState = READY_TO_GAME;
			SetLoading(2.0f);
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
			player->ChangeCamera(ENDING_GAME, WAITING_GAME);
			m_GameState = WAITING_GAME;
			break;
	}
	//std::cout << m_GameState << std::endl;
}

void GameState::ChangePrevState()
{
	Player* player = Input::GetInstance()->m_pPlayer;
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
			player->ChangeCamera(READY_TO_GAME, WAITING_GAME);
			m_GameState = WAITING_GAME;
			break;
		case PLAYING_GAME:
			m_GameState = READY_TO_GAME;
			break;
		case ENDING_GAME:
			m_GameState = PLAYING_GAME;
			break;
	}
	//std::cout << m_GameState << std::endl;
}

void GameState::ChangeSameLevelState()
{
	Player* player = Input::GetInstance()->m_pPlayer;
	std::cout << m_GameState << " -> ";
	switch (m_GameState) {
		case LOGIN:
			break;
		case ROOM_SELECT:
			break;
		case WAITING_GAME:
			player->ChangeCamera(WAITING_GAME, CUSTOMIZING);
			m_GameState = CUSTOMIZING;
			break;
		case CUSTOMIZING:
			player->ChangeCamera(CUSTOMIZING, WAITING_GAME);
			m_GameState = WAITING_GAME;
			break;
		case READY_TO_GAME:
			break;
		case PLAYING_GAME:
			break;
		case ENDING_GAME:
			break;
	}
	//std::cout << m_GameState << std::endl;
}
