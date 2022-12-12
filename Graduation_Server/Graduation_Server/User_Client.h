#pragma once
#include "stdafx.h"

enum STATE {ST_FREE, ST_ACCEPT, ST_INGAME};

class CLIENT {
private:
	char		_name[MAX_NAME_SIZE];
	STATE		_state;
	mutex		_state_lock;
	int			_prev_size;

	SOCKET		_socket;
	EXP_OVER	_recv_over;

public:
	CLIENT() : _state(ST_FREE), _prev_size(0)
	{

	}

	~CLIENT()
	{
		closesocket(_socket);
	}

	int		return_prev_size();

	STATE	get_state();
	void	set_state(STATE state);

	void	set_state_lock();
	void	set_state_unlock();

	void	set_prev_size(int _size);
	void	do_recv();
	void	do_send(int num_byte, void* mess);
};