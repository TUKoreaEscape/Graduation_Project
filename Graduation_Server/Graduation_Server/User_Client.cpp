#include "User_Client.h"

void CLIENT::set_prev_size(int _size)
{
	_prev_size = _size;
}

int CLIENT::return_prev_size()
{
	return _prev_size;
}

void CLIENT::do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over.m_wsa_over, sizeof(_recv_over.m_wsa_over));
	_recv_over.m_wsa_buf.buf = reinterpret_cast<char*>(_recv_over.m_buf + _prev_size);
	_recv_over.m_wsa_buf.len = sizeof(_recv_over.m_buf) - _prev_size;

	int ret = WSARecv(_socket, &_recv_over.m_wsa_buf, 1, 0, &recv_flag, &_recv_over.m_wsa_over, NULL);
	
	if (SOCKET_ERROR == ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			; // 에러디스플레이 만들거임
	}
}

void CLIENT::do_send(int num_byte, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_byte, mess);
	int ret = WSASend(_socket, &ex_over->m_wsa_buf, 1, 0, 0, &ex_over->m_wsa_over, NULL);
	
	if (SOCKET_ERROR == ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			; // 에러디스플레이 만들거임
	}
}

void CLIENT::set_state(STATE state)
{
	_state = state;
}

STATE CLIENT::get_state()
{
	return _state;
}

void CLIENT::set_state_lock()
{
	_state_lock.lock();
}

void CLIENT::set_state_unlock()
{
	_state_lock.unlock();
}