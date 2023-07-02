#pragma once
#include "stdafx.h"
#include "Room.h"

class RoomManager
{
private:
	array<Room, MAX_ROOM>	a_in_game_room;
	vector<GameObject>		m_game_object;
public:
	RoomManager();
	~RoomManager();

	void	init();
	void	init_object();

	int		Create_room(int user_id, int room_number);
	bool	Join_room(int user_id, int select_room_number);
	void	Clean_room(int room_number);
	void	Release();

	Room*	Get_Room_Info(int room_number);

	bool	collision_wall_player(int user_id);
	bool	collision_player_player(int user_id);

};