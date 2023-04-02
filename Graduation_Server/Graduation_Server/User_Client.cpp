#include "User_Client.h"

void CLIENT::set_prev_size(int _size)
{
	m_prev_size = _size;
}

int CLIENT::return_prev_size()
{
	return m_prev_size;
}

void CLIENT::do_recv()
{
	DWORD recv_flag = 0;

	ZeroMemory(&_recv_over.m_wsa_over, sizeof(_recv_over.m_wsa_over));
	_recv_over.m_wsa_buf.buf = reinterpret_cast<char*>(_recv_over.m_buf + m_prev_size);
	_recv_over.m_wsa_buf.len = sizeof(_recv_over.m_buf) - m_prev_size;

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
		}
	}
	delete ex_over;
}

void CLIENT::set_name(char* name)
{
	strcpy_s(m_name, MAX_NAME_SIZE, name);
}

void CLIENT::set_state(CLIENT_STATE::STATE state)
{
	m_state = state;
}

void CLIENT::set_login_state(LOGIN_STATE _state)
{
	m_login_state = _state;
}

void CLIENT::set_role(CLIENT_ROLE::STATE role)
{
	m_role = role;
}

void CLIENT::set_recv_over(EXP_OVER& recv_over, SOCKET c_socket)
{
	_recv_over.m_comp_op = recv_over.m_comp_op;
	_recv_over.m_wsa_buf.buf = recv_over.m_wsa_buf.buf;
	_recv_over.m_wsa_buf.len = recv_over.m_wsa_buf.len;
	ZeroMemory(&_recv_over.m_wsa_over, sizeof(recv_over.m_wsa_over));
	_socket = c_socket;
}

void CLIENT::set_join_room_number(int room_number)
{
	m_join_room_number = room_number;
}

void CLIENT::set_user_position(XMFLOAT3 pos)
{
	m_pos = pos;
}

void CLIENT::set_user_velocity(XMFLOAT3 velocity)
{
	m_velocity = velocity;
}

void CLIENT::set_user_yaw(float yaw)
{
	m_yaw = yaw;
}

void CLIENT::set_bounding_box(XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	m_bounding_box = BoundingOrientedBox{ center, extents, orientation }; // 임시값으로 오브젝트별 값을 따로 불러와서 적용 예정
}

void CLIENT::update_rotation(float yaw)
{
	m_yaw = yaw;

	float radian = XMConvertToRadians(yaw);

	XMFLOAT4 calculate{};
	XMStoreFloat4(&calculate, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = calculate;  // 바운딩박스도 회전은 해야지..
}

void CLIENT::update_bounding_box_pos(const XMFLOAT3 pos)
{
	m_bounding_box.Center = pos;
}

void CLIENT::update_bounding_box_orientation(const XMFLOAT4 orientation)
{
	m_bounding_box.Orientation = orientation;
}

BoundingOrientedBox CLIENT::get_bounding_box()
{
	return m_bounding_box;
}

XMFLOAT3 CLIENT::get_user_position()
{
	return m_pos;
}

XMFLOAT3 CLIENT::get_user_velocity()
{
	return m_velocity;
}

float CLIENT::get_user_yaw()
{
	return m_yaw;
}

CLIENT_STATE::STATE CLIENT::get_state()
{
	return m_state;
}

CLIENT_ROLE::STATE CLIENT::get_role()
{
	return m_role;
}

LOGIN_STATE CLIENT::get_login_state()
{
	return m_login_state;
}

void CLIENT::get_client_name(char& name, int size)
{
	strcpy_s(&name, size, m_name);
}

void CLIENT::error_display(int error_number)
{
	WCHAR* lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error_number, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, 0);

	std::wcout << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}