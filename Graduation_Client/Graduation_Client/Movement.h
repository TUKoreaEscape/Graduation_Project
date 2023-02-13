#pragma once
#include "Component.h"

class CommonMovement : public Component
{
public:
	void start();
	void update();
};

class TaggerMovement : public Component
{
public:
	void start() {};
	void update() {};
};

class SurvivorMovement : public Component
{
public:
	void start() {};
	void update() {};
};

class DeadMovement : public Component
{
public:
	void start() {};
	void update() {};
};