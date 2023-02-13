#pragma once
#include "Scene.h"
#include "Player.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //게임이 진행중인지 끝났는지 사용하기위한 변수이다. 아직은 사용하지 않고 정의만 해두었다.

class GameScene : public Scene
{
public:
	class Player* player;
public:
	GameScene();
	~GameScene() {}

	virtual void render();
};
