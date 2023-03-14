#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "EXPOver.h"

class C_IOCP
{
private:
	void Bind_Socket(short port_num);

public:
	C_IOCP(){};
	~C_IOCP(){};

public:
	void Start_server();
	void Close_server();

	void Disconnect();

public:
	HANDLE		m_h_iocp{};
	SOCKET		m_client_socket{};
	SOCKET		m_listen_socket{};
	EXP_OVER	m_exp_over{};
};