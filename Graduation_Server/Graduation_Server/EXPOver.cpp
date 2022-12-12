#pragma once
#include "EXPOver.h"

EXP_OVER::EXP_OVER(COMP_OP comp_op, char _size, void* message) : m_comp_op(comp_op)
{
	ZeroMemory(&m_wsa_buf, sizeof(m_wsa_over));
	m_wsa_buf.buf = reinterpret_cast<char*>(m_buf);
	m_wsa_buf.len = _size;
	memcpy(m_buf, message, _size);
}

EXP_OVER::EXP_OVER(COMP_OP comp_op) : m_comp_op(comp_op)
{

}

EXP_OVER::EXP_OVER()
{
	m_comp_op = OP_RECV;
}

EXP_OVER::~EXP_OVER()
{

}