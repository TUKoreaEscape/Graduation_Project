#pragma once
#include "Component.h"

class CommonMovement : public Component
{
public:
	void start();
	void update(float elapsedTime);
};

class TaggerMovement : public Component
{
public:
	void start() {};
	void update(float elapsedTime) {};
};

class SurvivorMovement : public Component
{
public:
	void start() {};
	void update(float elapsedTime) {};
};

class DeadMovement : public Component
{
public:
	void start() {};
	void update(float elapsedTime) {};
};