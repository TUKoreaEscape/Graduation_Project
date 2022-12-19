#pragma once
#include "stdafx.h"



class Room {
private:
	int		room_number;
	int		Number_of_users;
	int		remain_user;

	array<int, 6> in_player; // 방에 들어온 플레이어 id

public:
	GAME_ROOM_STATE::TYPE _room_state;

public:
	Room() : _room_state(GAME_ROOM_STATE::TYPE::NONE)
	{

	}

	~Room()
	{

	}
};