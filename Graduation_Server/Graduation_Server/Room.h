#pragma once
#include "stdafx.h"
#include "object.h"
#include "User_Client.h"

enum Room_State{ROOM_FREE, ROOM_READY, ROOM_GAMING};
//class CLIENT;
class Room {
private:
	int					room_number;
	int					Number_of_users = 0;
	int					remain_user = 6;
	unsigned char		m_room_name[MAX_NAME_SIZE] {"test\0"};




	array<BoundingOrientedBox, 6>	in_player_bounding_box;
	vector<GameObject>				m_game_object;

public:
	GAME_ROOM_STATE::TYPE	_room_state;
	array<int, 6>			in_player; // 방에 들어온 플레이어 id XMFLOAT3 
public:
	Room()
	{
		_room_state = GAME_ROOM_STATE::FREE;
		in_player.fill(-1);
	}

	~Room()
	{

	}

	void init_room_number(const int room_num)
	{
		room_number = room_num;
	}

	void	add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	SetBoundingBox(XMFLOAT3 pos);


	void	Reset_Room();
	void	Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	bool	Join_Player(int user_id);
	void	Exit_Player(int user_id);

	int		Get_Number_of_users();

	int		Get_Join_Member(int data);
	char*	Get_Room_Name(char room_name[], int size);
};