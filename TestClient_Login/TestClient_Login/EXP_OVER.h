#pragma once
#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "protocol.h"
#include <map>



class EXP_OVER {
public:
	WSAOVERLAPPED	m_wsa_over;
	COMP_OP			m_comp_op;
	WSABUF			m_wsa_buf;
	unsigned char	m_buf[BUFSIZE];

public:
	EXP_OVER(COMP_OP comp_op, char num_byte, void* message);
	EXP_OVER(COMP_OP comp_op);
	EXP_OVER();

	~EXP_OVER();
};