#pragma once
#include "stdafx.h"

namespace CLIENT_STATE
{
	enum STATE {
		ST_FREE, ST_ACCEPT, ST_LOBBY, ST_GAMEROOM, ST_INGAME
	};
}

namespace CLIENT_ROLE
{
	enum STATE {
		ROLE_NONE, ROLE_RUNNER, ROLE_TAGGER
	};
}

enum LOGIN_STATE { N_LOGIN, Y_LOGIN };

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

	BoundingOrientedBox		m_bounding_box{};

public:
	unordered_set <int> room_list;
	unordered_set <int> view_list; // 현재는 사용하지 않지만 맵을 서버에 추가할 때 사용할 예정

	mutex		_room_list_lock;
	mutex		_state_lock;

	SOCKET		_socket = NULL;
	EXP_OVER	_recv_over;

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

	void				set_login_state(LOGIN_STATE _state);
	void				set_state(CLIENT_STATE::STATE state);
	void				set_role(CLIENT_ROLE::STATE role);
	void				set_id(int id) { m_id = id; }
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

};