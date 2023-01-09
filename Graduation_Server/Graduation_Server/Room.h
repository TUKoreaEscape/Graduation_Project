#pragma once
#include "stdafx.h"



class Room {
private:
	int		room_number;
	int		Number_of_users = 0;
	int		remain_user = 6;

	array<int, 6> in_player; // 방에 들어온 플레이어 id

public:
	GAME_ROOM_STATE::TYPE _room_state;

public:
	Room(int make_player_id) : _room_state(GAME_ROOM_STATE::TYPE::NONE)
	{
		in_player[0] = make_player_id;
		Number_of_users++;
		remain_user--;
	}

	~Room()
	{

	}
};