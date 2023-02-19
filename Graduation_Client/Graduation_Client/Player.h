#pragma once
#include "GameObject.h"
#include "Movement.h"

class Player : public GameObject
{
public:
	Player() : GameObject()
	{
		AddComponent<CommonMovement>();
	}
	void render();
};