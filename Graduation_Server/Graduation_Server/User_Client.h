#pragma once
#include "stdafx.h"

enum STATE {ST_FREE, ST_ACCEPT, ST_LOBBY, ST_GAMEROOM, ST_INGAME};
enum LOGIN_STATE {N_LOGIN, Y_LOGIN};
class CLIENT {
private:
	char					_name[MAX_NAME_SIZE];
	STATE					_state;
	LOGIN_STATE				_login_state;

	unsigned short			_prev_size;
	int						_id;
	int						_join_room_number;

	XMFLOAT3				m_pos;
	BoundingOrientedBox		m_bounding_box;

public:
	unordered_set <int> room_list; // 원래는 같은방에 누가 있는지만 저장하려 했지만 room 클래스 내부에 in_player가 누가 있는지 저장하므로 중복역할...ㅠ
	unordered_set <int> view_list;

	mutex		_room_list_lock;
	mutex		_state_lock;

	SOCKET		_socket = NULL;
	EXP_OVER	_recv_over;

public:
	CLIENT() : _id(-1), _state(ST_FREE), _prev_size(0), _login_state(N_LOGIN), _join_room_number(-1)
	{

	}

	~CLIENT()
	{
		closesocket(_socket);
	}

	int					return_prev_size();
	int					get_id() { return _id; }
	int					get_join_room_number() { return _join_room_number; }

	STATE				get_state();
	LOGIN_STATE			get_login_state();

	void				set_login_state(LOGIN_STATE _state);
	void				set_state(STATE state);
	void				set_id(int id) { _id = id; }
	void				set_recv_over(EXP_OVER& exp_over, SOCKET c_socket);
	void				set_join_room_number(int room_number);

	void				error_display(int error_number);

	void				set_prev_size(int _size);
	void				do_recv();
	void				do_send(int num_byte, void* mess);

	void				set_user_position(XMFLOAT3 pos);
	XMFLOAT3			get_user_position();

	void				set_bounding_box(XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	BoundingOrientedBox get_bounding_box();
};