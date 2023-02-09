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
		if (ERROR_IO_PENDING != error_num) {
			error_display(error_num);
			cout << "recv error" << endl;
		}
	}
}

void CLIENT::do_send(int num_byte, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_byte, mess);
	int ret = WSASend(_socket, &ex_over->m_wsa_buf, 1, 0, 0, &ex_over->m_wsa_over, NULL);
	
	if (SOCKET_ERROR == ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num) {
			error_display(error_num);
			cout << "전송실패" << endl;
		}
	}
	delete ex_over;
	cout << "전송완료" << endl;
}

void CLIENT::set_state(STATE state)
{
	_state = state;
}

STATE CLIENT::get_state()
{
	return _state;
}


void CLIENT::set_recv_over(EXP_OVER& recv_over, SOCKET c_socket)
{
	_recv_over.m_comp_op = recv_over.m_comp_op;
	_recv_over.m_wsa_buf.buf = recv_over.m_wsa_buf.buf;
	_recv_over.m_wsa_buf.len = recv_over.m_wsa_buf.len;
	ZeroMemory(&_recv_over.m_wsa_over, sizeof(recv_over.m_wsa_over));
	_socket = c_socket;
}

void CLIENT::error_display(int error_number)
{
	WCHAR* lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error_number, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, 0);

	std::wcout << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}