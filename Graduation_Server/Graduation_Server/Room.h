#pragma once
#include "stdafx.h"

enum Room_State{ROOM_FREE, ROOM_READY, ROOM_GAMING};
class CLIENT;
class Room {
private:
	int			room_number;
	int			Number_of_users = 0;
	int			remain_user = 6;
	char		m_room_name[MAX_NAME_SIZE] {"test"};

	array<int, 6> in_player; // �濡 ���� �÷��̾� id XMFLOAT3 
	

public:
	GAME_ROOM_STATE::TYPE _room_state;

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

	void SetBoundingBox(CLIENT& cl)
	{
		// �浹üũ�� ���� dx12�� �߰��� ��뿹��
		
	}


	void Reset_Room();
	void Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	bool Join_Player(int user_id);
	void Exit_Player(int user_id);

	int Get_Number_of_users();
	char* Get_Room_Name(char room_name[], int size);
};