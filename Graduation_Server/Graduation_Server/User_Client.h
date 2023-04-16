#pragma once
#include "stdafx.h"
#include <codecvt>
#include "User_Custom.h"
namespace CLIENT_STATE
{
	enum STATE {
		ST_FREE, ST_ACCEPT, ST_LOBBY, ST_GAMEROOM, ST_INGAME
	};
}

namespace CLIENT_ROLE
{
	enum STATE {
		ROLE_NONE, ROLE_RUNNER,  ROLE_TAGGER
	};
}

enum LOGIN_STATE {N_LOGIN, Y_LOGIN};

class CLIENT {
private:
	char					m_name[MAX_NAME_SIZE]{};

	LOGIN_STATE				m_login_state = N_LOGIN;
	CLIENT_STATE::STATE		m_state = CLIENT_STATE::ST_FREE;
	CLIENT_ROLE::STATE		m_role = CLIENT_ROLE::ROLE_NONE;

	unsigned short			m_prev_size;
	int						m_id;
	int						m_join_room_number;

	XMFLOAT3				m_pos{};
	XMFLOAT3				m_velocity{};
	float					m_yaw{};
	short					m_look[3];
	short					m_right[3];
	unsigned char			m_input_key;
	BoundingOrientedBox		m_bounding_box{};

private:
	bool					m_first_skill_able = false;
	bool					m_second_skill_able = false;
	bool					m_third_skill_able = false;

public:
	unordered_set <int> room_list; 
	unordered_set <int> view_list; // 현재는 사용하지 않지만 맵을 서버에 추가할 때 사용할 예정

	mutex		_room_list_lock;
	mutex		_state_lock;
	mutex		_update_lock;

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

	void				set_look(Look look) { m_look[0] = look.x; m_look[1] = look.y; m_look[2] = look.z; }
	void				set_right(Right right) { m_right[0] = right.x; m_right[1] = right.y; m_right[2] = right.z; }
	void				set_inputKey(unsigned char key) { m_input_key = key; }

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

};