#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "protocol.h"
#include <map>

enum OP_TYPE { 
	OP_RECV, 
	OP_SEND, 
	OP_ACCEPT, 
	OP_NPC_MOVE, 
	OP_PLAYER_MOVE, 
	OP_DOOR_OPEN, 
	OP_DOOR_CLOSE, 
	OP_UPDATE_PLAYER_MOVE,
	OP_PLAYER_ATTACK,
	OP_PLAYER_VICTIM,
	OP_SELECT_TAGGER,
	OP_FIRST_TAGGER_SKILL_OPEN,
	OP_SECOND_TAGGER_SKILL_OPEN,
	OP_THIRD_TAGGER_SKILL_OPEN,
	OP_GAME_END,
	OP_SERVER_END
};

class EXP_OVER {
public:
	WSAOVERLAPPED	m_wsa_over{};
	OP_TYPE			m_comp_op{};
	WSABUF			m_wsa_buf{};
	unsigned char	m_buf[BUFSIZE]{};

public:
	EXP_OVER(OP_TYPE comp_op, int num_byte, void* message);
	EXP_OVER(OP_TYPE comp_op);
	EXP_OVER();

	~EXP_OVER();
};