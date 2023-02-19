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

	void	init();
	int		Create_room(int user_id);
	void	Join_room(int user_id, int select_room_number);

	bool	collision_wall_player(int user_id);
	bool	collision_player_player(int user_id);

};