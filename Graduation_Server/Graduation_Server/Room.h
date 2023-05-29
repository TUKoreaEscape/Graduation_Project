#pragma once
#include "stdafx.h"
#include "object.h"
#include "User_Client.h"

#define GAME_END_COLLECT_CHIP 12

//class CLIENT;
class Room {
private:
	int					room_number;
	int					Number_of_users = 0;
	int					remain_user = 6;
	char				m_room_name[MAX_NAME_SIZE] {"test\0"};
	int					m_tagger_id = -1;
	long long			duration_time;

	array<BoundingOrientedBox, 6>	in_player_bounding_box;
	vector<GameObject>				m_game_object;
	vector<GameObject>				m_game_wall_and_fix_object;
	vector<Door>					m_door_object;

	chrono::system_clock::time_point start_time;
	chrono::system_clock::time_point now_time;

private: // ���� �ΰ��� �÷��̽� ����ϴ� ����
	bool				m_first_skill_enable = false;
	bool				m_second_skill_enable = false;
	bool				m_third_skill_enable = false;

	int					m_tagger_collect_chip = 0;

public:
	mutex					_room_state_lock;
	GAME_ROOM_STATE::TYPE	_room_state;

	array<int, 6>			in_player; // �濡 ���� �÷��̾� id XMFLOAT3 
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

	// �ΰ��� �ε� �� ����ϴ� �Լ�
	void	add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	add_game_walls(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);
	void	add_game_doors(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);
	void	Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	void	SetReady(const bool is_ready, const int user_id);
	void	SetLoading(const bool is_loading, const int user_id);
	void	Exit_Player(int user_id);

public:
	// �ΰ��� ������ ����ϴ� �Լ�
	void	SetBoundingBox(XMFLOAT3 pos, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	SetPlayerPos(CLIENT& player);
	void	Update_room_time();
	void	Update_Player_Position();
	void	Update_Door(const int door_num);

public: // �ΰ��� ������Ʈ state �޾ƾ��ϴ� ����
	Door_State Get_Door_State(const int door_num) { return m_door_object[door_num].get_state(); }

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

	bool	Is_Door_Open(const int door_num);
	CollisionInfo	is_collision_wall_to_player(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift);
	CollisionInfo	is_collision_player_to_player(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift);
	CollisionInfo	is_collision_player_to_object(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift);

	char*	Get_Room_Name(char room_name[], int size);
};