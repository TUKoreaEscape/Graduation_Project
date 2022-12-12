#pragma once


// 클라이언트와 서버간 통신에 사용할 구조체를 정의합니다.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 256;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 30;
// ----- 클라이언트가 서버에게 보낼때 ------
enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT, OP_NPC_MOVE, OP_PLAYER_MOVE };
namespace CS_PACKET
{
	enum TYPE
	{
		NONE = 0,
		CS_LOGIN,
		CS_MOVE,
		CS_PACKET_CHAT
	};
}


struct cs_packet_login {
	unsigned char size;
	unsigned char type;
};

struct cs_packet_move {
	unsigned char size;
	unsigned char type;
};

struct cs_packet_voice {
	unsigned char size;
	unsigned char type;

};

struct cs_packet_chat {
	unsigned char	size;
	char			type;
	char			message[MAX_CHAT_SIZE];
};

// ----- 서버가 클라이언트에게 보낼때 -----

namespace SC_PACKET
{
	enum TYPE
	{
		NONE = 0,
		SC_LOGINOK,
		SC_LOGINFAIL,
		SC_MOVING,
		SC_PACKET_CHAT
	};
}

struct sc_packet_login_ok {
	unsigned char	size;
	unsigned char	type;
	unsigned int	id;
};

struct sc_packet_login_fail {

};

struct sc_packet_move {

};

struct sc_packet_create_room {
	unsigned char size;
	unsigned char type;

	// 방 생성 내용
};

struct sc_packet_user_join_room {
	unsigned char size;
	unsigned char type;

	// 유저 방 입장시 room manager에 데이터 이동
};

struct sc_packet_chat {
	unsigned char	size;
	char			type;
	int				id;
	char			message[MAX_CHAT_SIZE];
};