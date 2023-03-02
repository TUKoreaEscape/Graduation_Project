#pragma once
#include "stdafx.h"

enum Room_State{ROOM_FREE, ROOM_READY, ROOM_GAMING};
class CLIENT;
class Room {
private:
	int			room_number = -1;
	int			Number_of_users = 0;
	int			remain_user = 6;
	char		m_room_name[MAX_NAME_SIZE] {"test"};

	array<int, 6> in_player{ -1 }; // 방에 들어온 플레이어 id
	
public:
	GAME_ROOM_STATE::TYPE _room_state;

public:
	Room()
	{
		_room_state = GAME_ROOM_STATE::FREE;
	}

	~Room()
	{

	}

	void SetBoundingBox(CLIENT& cl)
	{
		// 충돌체크를 위해 dx12를 추가후 사용예정
	}


	void Reset_Room();
	void Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	void Join_Player(int user_id);
	void Exit_Player(int user_id);

	int Get_Number_of_users();
	char* Get_Room_Name(char room_name[], int size);
};