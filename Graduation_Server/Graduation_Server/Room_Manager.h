#pragma once
#include "stdafx.h"
#include "Room.h"

class RoomManager
{
private:
	//array<Room, 5000> in_game_room;
	vector<Room> in_game_room;

public:
	RoomManager();
	~RoomManager();

	void init();
	void Create_room();
};