#pragma once
#include "stdafx.h"
#include "object.h"
#include "User_Client.h"

#define GAME_END_COLLECT_CHIP 12
#define JOIN_ROOM_MAX_USER 6

//#define FIRST_TAGGER_SKILL_OPEN_SECOND 180
//#define SECOND_TAGGER_SKILL_OPEN_SECOND 360
//#define THIRD_TAGGER_SKILL_OPEN_SECOND 540
//#define GAME_END_SECOND 900

#define FIRST_TAGGER_SKILL_OPEN_SECOND 10
#define SECOND_TAGGER_SKILL_OPEN_SECOND 15
#define THIRD_TAGGER_SKILL_OPEN_SECOND 20
#define GAME_END_SECOND 25


//class CLIENT;
class Room {
private:
	int					room_number;
	int					Number_of_users = 0;
	int					remain_user = 6;
	char				m_room_name[MAX_NAME_SIZE] {"test\0"};
	int					m_tagger_id = -1;
	long long			duration_time;

	char room_name1[10]{ "Tagger?" };
	char room_name2[10]{ "NowHere!" };
	char room_name3[10]{ "TO DO!" };
	char room_name4[10]{ "Plz" };
	char room_name5[10]{ "KeepMovin" };
	char room_name6[10]{ "Escape!" };
	char room_name7[10]{ "Beat" };
public:
	array<BoundingOrientedBox, 6>	in_player_bounding_box;
	vector<GameObject>				m_game_object;
	vector<GameObject>				m_game_fix_object;
	vector<GameObject>				m_game_wall;
	vector<Door>					m_door_object;
	vector<Vent>					m_vent_object;
	vector<ElectronicSystem>		m_electrinic_system;
	vector<EscapeSystem>			m_escape_system;
	vector<GameItem>				m_fix_item;
	Altar*							m_altar = nullptr;

private:
	chrono::system_clock::time_point start_time;
	chrono::system_clock::time_point now_time;

private: // 여긴 인게임 플레이시 사용하는 변수
	bool				m_first_skill_enable = false;
	bool				m_second_skill_enable = false;
	bool				m_third_skill_enable = false;
	bool				m_tagger_now_collect_life_chip = false;
	int					m_tagger_collect_chip = 0;

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
		m_altar = new Altar;
		for (int i = 0; i < 3; ++i) {
			m_escape_system.emplace_back();
			m_escape_system[i].init();
		}

	}

	~Room()
	{
		delete m_altar;
	}
public:
	void	init_room_by_game_end(); // 방 초기화

public: // 서버 시작시 초기화하는 함수들
	void	init_room_number(const int room_num){ room_number = room_num;}
	void	init_room_name(const int rand_num);
	void	init_fix_object_and_life_chip();

	void	add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	add_game_walls(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);
	void	add_game_doors(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);
	void	add_game_vents(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);
	void	add_game_ElectronicSystem(const unsigned int id, Object_Type ob_type, XMFLOAT3& center, XMFLOAT3& extents);
	void	add_fix_objects(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents);

public: // 게임 시작 전 사용하는 함수들
	void	Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state);
	void	SetReady(const bool is_ready, const int user_id);
	void	SetLoading(const bool is_loading, const int user_id);
	void	Exit_Player(int user_id);
	bool	Join_Player(int user_id);
	bool	All_Player_Ready();
	bool	All_Player_Loading();

public:
	// 인게임 시작후 사용하는 함수
	void	SetBoundingBox(XMFLOAT3 pos, XMFLOAT3 extents, XMFLOAT4 orientation);
	void	Set_Electronic_System_ONOFF();
	void	Update_room_time();
	void	Update_Player_Position();
	void	Update_Door(const int door_num);
	void	Update_ElectronicSystem_Door(const int es_num, bool value);

public: // 술래 스킬 사용 관련 함수들
	void	Tagger_Use_First_Skill();
	void	Tagger_Use_Second_Skill(int room_number);
	void	Tagger_Use_Third_Skill();

public:
	void	Tagger_Get_Life_Chip(bool value) { m_tagger_collect_chip = value; }
	bool	Is_Tagger_Get_Life_Chip() { return m_tagger_collect_chip; }
	bool	Is_Tagger_Winner();

public: // 인게임 오브젝트 state 받아야하는 공간
	Door_State Get_Door_State(const int door_num) { return m_door_object[door_num].get_state(); }
	ES_State   Get_EletronicSystem_State(const int es_num) { return m_electrinic_system[es_num].get_state(); }

public: // 게임 state 변환하는 함수
	void	Reset_Room();
	void	Start_Game();
	void	End_Game(bool is_tagger_win);

public: // 인게임 아이템관련 함수
	void	Activate_Altar();
	bool	All_ElectronicSystem_Fixed();
	bool	Pick_Item(const int item_type);
	bool	Is_near(XMFLOAT3 player_pos, XMFLOAT3 object_pos, int range);

public:
	int		Select_Tagger();
	int		Get_Number_of_users();
	int		Get_Join_Member(int data);
	int		Get_Tagger_ID() { return m_tagger_id; }

	bool	Is_Door_Open(const int door_num);
	bool	Is_ElectronicSystem_Open(const int es_num);

public: // 충돌 관련 함수
	CollisionInfo	is_collision_wall_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);
	CollisionInfo	is_collision_player_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);
	CollisionInfo	is_collision_fix_object_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);
	CollisionInfo	is_collision_player_to_object(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);
	CollisionInfo	is_collision_player_to_door(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);
	CollisionInfo	is_collision_player_to_vent(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift);

	char*	Get_Room_Name(char room_name[], int size);
};