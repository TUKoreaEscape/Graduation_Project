#pragma once
#include "Component.h"
#include "Player.h"
#include "GameObject.h"

class CommonMovement : public Component
{
public:
	Player* m_pPlayer = nullptr;

	void start();
	void update(float elapsedTime);
};

class TaggerMovement : public Component
{
public:
	Player* m_pPlayer = nullptr;
	void start() {};
	void update(float elapsedTime) {};
};

class SurvivorMovement : public Component
{
public:
	Player* m_pPlayer = nullptr;
	void start() {};
	void update(float elapsedTime) {};
};

class DeadMovement : public Component
{
public:
	Player* m_pPlayer = nullptr;
	void start() {};
	void update(float elapsedTime) {};
};