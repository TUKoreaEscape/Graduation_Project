#pragma once
#include "stdafx.h"
#include <codecvt>
#include "User_Custom.h"

enum LOGIN_STATE {N_LOGIN, Y_LOGIN};

class CLIENT {
private:
	char					m_name[MAX_NAME_SIZE]{};

	LOGIN_STATE				m_login_state = N_LOGIN;
	CLIENT_STATE::STATE		m_state = CLIENT_STATE::ST_FREE;
	CLIENT_ROLE::STATE		m_role = CLIENT_ROLE::ROLE_NONE;

	unsigned short			m_prev_size{};
	int						m_id{};
	int						m_join_room_number{};
	int						m_look_lobby_page = -1;

	XMFLOAT3				m_pos{};
	XMFLOAT3				m_velocity{};
	bool					m_jump = false;
	bool					m_collied_up_face = false;
	bool					m_attack_animation = false;
	bool					m_victim_animation = false;

	float					m_attack_animation_time = 0;
	float					m_victim_animation_time = 0;

	float					m_yaw{};
	short					m_look[3]{0};
	short					m_right[3]{0};
	unsigned char			m_input_key{};
	BoundingOrientedBox		m_bounding_box{};

public:
	bool					m_is_stresstest_npc = false;
	bool					m_befor_send_move = false;

private:
	bool					m_first_skill_able = false;
	bool					m_second_skill_able = false;
	bool					m_third_skill_able = false;

private: // 아이템 부분
	bool					m_life_card_own = false;
	bool					m_item_own[4]{ false };

public:
	unordered_set <int> room_list; 
	unordered_set <int> view_list; // 현재는 사용하지 않지만 맵을 서버에 추가할 때 사용할 예정

	mutex		_room_list_lock;
	mutex		_view_list_lock;
	mutex		_state_lock;
	mutex		_update_lock;
	mutex		_pos_lock;

	SOCKET		_socket = NULL;
	EXP_OVER	_recv_over;

	Customizing_Info* m_customizing = nullptr;
public:
	CLIENT() : m_id(-1), m_state(CLIENT_STATE::ST_FREE), m_prev_size(0), m_login_state(N_LOGIN), m_join_room_number(-1)
	{

	}

	~CLIENT()
	{
		closesocket(_socket);
	}

	int					return_prev_size();
	int					get_id() { return m_id; }
	int					get_join_room_number() { return m_join_room_number; }
	int					get_look_lobby_page() { return m_look_lobby_page; }
	bool				get_user_is_jump() { return m_jump; }
	bool				get_user_collied_up_face() { return m_collied_up_face; }
	bool				get_user_attack_animation() { return m_attack_animation; }
	bool				get_user_victim_animation() { return m_victim_animation; }

	float				get_user_yaw();
	XMFLOAT3			get_user_position();
	XMFLOAT3			get_user_velocity();
	LOGIN_STATE			get_login_state();
	CLIENT_STATE::STATE	get_state();
	CLIENT_ROLE::STATE	get_role();
	BoundingOrientedBox get_bounding_box();
	void				get_client_name(char& name, int size);
	
	short				get_look_x() { return m_look[0]; }
	short				get_look_y() { return m_look[1]; }
	short				get_look_z() { return m_look[2]; }
	short				get_right_x() { return m_right[0]; }
	short				get_right_y() { return m_right[1]; }
	short				get_right_z() { return m_right[2]; }
	unsigned char		get_input_key() { return m_input_key; }

	void				set_look_lobby_page(int page) { m_look_lobby_page = page; }

	void				set_look(Look look) { m_look[0] = look.x; m_look[1] = look.y; m_look[2] = look.z; }
	void				set_right(Right right) { m_right[0] = right.x; m_right[1] = right.y; m_right[2] = right.z; }
	void				set_inputKey(unsigned char key) { m_input_key = key; }
	void				set_isjump(bool value) { m_jump = value; }
	void				set_collied_up_face(bool value) { m_collied_up_face = value; }
	void				set_attack_animation(bool value) { m_attack_animation = value; }
	void				set_victim_animation(bool value) { m_victim_animation = value; }

	bool				IsAttackAnimation() { return m_attack_animation; }
	bool				IsVictimAnimation() { return m_victim_animation; }

	void				set_login_state(LOGIN_STATE _state);
	void				set_state(CLIENT_STATE::STATE state);
	void				set_role(CLIENT_ROLE::STATE role);
	void				set_id(int id);
	void				set_name(char* name);
	void				set_recv_over(EXP_OVER& exp_over, SOCKET c_socket);
	void				set_join_room_number(int room_number);
	void				set_prev_size(int _size);
	void				set_bounding_box(XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void				set_user_position(XMFLOAT3 pos);
	void				set_user_velocity(XMFLOAT3 velocity);
	void				set_user_yaw(float yaw);

	void				update_rotation(float yaw);
	void				update_bounding_box_pos(const XMFLOAT3 pos);
	void				update_bounding_box_orientation(const XMFLOAT4 orientation);
	void				error_display(int error_number);

	// 아이템 관련 함수

	void				set_item_own(GAME_ITEM::ITEM item, bool value) { m_item_own[item] = value; }
	bool				get_item_own(GAME_ITEM::ITEM item) { return m_item_own[item]; }

	

	// 네트워크용 함수 2개
	void				do_recv();
	void				do_send(int num_byte, void* mess);

	wstring				stringToWstring(const std::string& t_str)
	{
		typedef codecvt_utf8<wchar_t>  convert_type;
		wstring_convert<convert_type, wchar_t> converter;

		return converter.from_bytes(t_str);
	}

public:
	void				set_first_skill_enable() { m_first_skill_able = true; };
	void				set_second_skill_enable() { m_second_skill_able = true; };
	void				set_third_skill_enable() { m_third_skill_able = true; };

	void				use_first_skill() { if (m_first_skill_able == true) m_first_skill_able = false; }
	void				use_second_skill() { if (m_second_skill_able == true) m_second_skill_able = false; }
	void				use_third_skill() { if (m_third_skill_able == true) m_third_skill_able = false; }

	bool				get_first_skill_enable() { return m_first_skill_able; }
	bool				get_second_skill_enable() { return m_second_skill_able; }
	bool				get_third_skill_enable() { return m_third_skill_able; }
};