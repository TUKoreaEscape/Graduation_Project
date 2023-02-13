#pragma once
#include "GameScene.h"

E_GAME_STATE gameState;

GameScene::GameScene() : Scene()
{
	player = new Player();
	gameState = E_GAME_RUNNING;
}

void GameScene::render()
{
	Scene::render();
}