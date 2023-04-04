#pragma once
#include "stdafx.h"
#include "object.h"
#include "User_Client.h"


//class CLIENT;
class Room {
private:
	int					room_number;
	int					Number_of_users = 0;
	int					remain_user = 6;
	char				m_room_name[MAX_NAME_SIZE]{ "test\0" };
	int					m_tagger_id = -1;
	long long			duration_time;

	array<BoundingOrientedBox, 6>	in_player_bounding_box;
	vector<GameObject>				m_game_object;

	chrono::system_clock::time_point start_time;
	chrono::system_clock::time_point now_time;

public:
	mutex					_room_state_lock;
	GAME_ROOM_STATE::TYPE	_room_state;

	array<int, 6>			in_player; // 방에 들어온 플레이어 id XMFLOAT3 
	array<bool, 6>			in_player_ready;
	array<bool, 6>			in_player_loading_success;
	mutex					in_player_lock;

public:
	Room()
	{
		_room_state = GAME_ROOM_STATE::FREE;
		in_player.fill(-1);
		in_player_ready.fill(false);
	}

	~Room()
	{

	}

	void init_room_number(const int room_num)
	{
		room_number = room_num;
	}

	// 인게임 로딩 전 사용하는 함수
	void	add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	void	SetReady(const bool is_ready, const int user_id);
	void	SetLoading(const bool is_loading, const int user_id);
	void	Exit_Player(int user_id);

public:
	// 인게임 시작후 사용하는 함수
	void	SetBoundingBox(XMFLOAT3 pos, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	SetPlayerPos(CLIENT& player);
	void	Update_room_time();

public:
	void	Reset_Room();
	void	Start_Game();
	void	End_Game();
private:
	int		Select_Tagger();

public:
	int		Get_Number_of_users();
	int		Get_Join_Member(int data);

	bool	Join_Player(int user_id);
	bool	All_Player_Ready();
	bool	All_Player_Loading();

	bool	Is_Door_Open();
	bool	is_collision_wall_to_player(const int player_id);
	bool	is_collision_player_to_player(const int player_id);
	bool	is_collision_player_to_object(const int player_id);

	char* Get_Room_Name(char room_name[], int size);
};