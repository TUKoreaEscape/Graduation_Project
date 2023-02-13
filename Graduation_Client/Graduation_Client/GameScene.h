#pragma once
#include "Scene.h"
#include "Player.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //������ ���������� �������� ����ϱ����� �����̴�. ������ ������� �ʰ� ���Ǹ� �صξ���.

class GameScene : public Scene
{
public:
	class Player* player;
public:
	GameScene();
	~GameScene() {}

	virtual void render();
};
