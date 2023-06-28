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