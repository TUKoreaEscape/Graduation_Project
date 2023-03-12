#pragma once
#include "stdafx.h"

namespace CLIENT_STATE
{
	enum STATE {
		ST_FREE, ST_ACCEPT, ST_LOBBY, ST_GAMEROOM, ST_INGAME
	};
}
enum LOGIN_STATE {N_LOGIN, Y_LOGIN};

class CLIENT {
private:
	char					_name[MAX_NAME_SIZE];
	LOGIN_STATE				_login_state;
	CLIENT_STATE::STATE		_state;

	unsigned short			_prev_size;
	int						_id;
	int						_join_room_number;

	XMFLOAT3				m_pos;
	XMFLOAT3				m_velocity;
	float					m_yaw;

	BoundingOrientedBox		m_bounding_box;

public:
	unordered_set <int> room_list; 
	unordered_set <int> view_list; // 현재는 사용하지 않지만 맵을 서버에 추가할 때 사용할 예정

	mutex		_room_list_lock;
	mutex		_state_lock;

	SOCKET		_socket = NULL;
	EXP_OVER	_recv_over;

public:
	CLIENT() : _id(-1), _state(CLIENT_STATE::ST_FREE), _prev_size(0), _login_state(N_LOGIN), _join_room_number(-1)
	{

	}

	~CLIENT()
	{
		closesocket(_socket);
	}

	int					return_prev_size();
	int					get_id() { return _id; }
	int					get_join_room_number() { return _join_room_number; }
	float				get_user_yaw();
	XMFLOAT3			get_user_position();
	XMFLOAT3			get_user_velocity();
	LOGIN_STATE			get_login_state();
	CLIENT_STATE::STATE	get_state();
	BoundingOrientedBox get_bounding_box();
	void				get_client_name(char& name, int size);

	void				set_login_state(LOGIN_STATE _state);
	void				set_state(CLIENT_STATE::STATE state);
	void				set_id(int id) { _id = id; }
	void				set_name(char* name);
	void				set_recv_over(EXP_OVER& exp_over, SOCKET c_socket);
	void				set_join_room_number(int room_number);
	void				set_prev_size(int _size);
	void				set_bounding_box(XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	void				set_user_position(XMFLOAT3 pos);
	void				set_user_velocity(XMFLOAT3 velocity);
	void				set_user_yaw(float yaw);

	void				error_display(int error_number);
	
	void				do_recv();
	void				do_send(int num_byte, void* mess);

};