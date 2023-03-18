#pragma once
#include "Component.h"
#include "Player.h"

class CommonMovement : public Component
{
	Player* m_pPlayer = nullptr;
public:
	void start();
	void update(float elapsedTime);
};

class TaggerMovement : public Component
{
	Player* m_pPlayer = nullptr;
public:
	void start() {};
	void update(float elapsedTime) {};
};

class SurvivorMovement : public Component
{
	Player* m_pPlayer = nullptr;
public:
	void start() {};
	void update(float elapsedTime) {};
};

class DeadMovement : public Component
{
	Player* m_pPlayer = nullptr;
public:
	void start() {};
	void update(float elapsedTime) {};
};