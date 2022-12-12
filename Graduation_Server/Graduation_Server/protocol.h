#pragma once


// Ŭ���̾�Ʈ�� ������ ��ſ� ����� ����ü�� �����մϴ�.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 256;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 30;
// ----- Ŭ���̾�Ʈ�� �������� ������ ------
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

// ----- ������ Ŭ���̾�Ʈ���� ������ -----

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

	// �� ���� ����
};

struct sc_packet_user_join_room {
	unsigned char size;
	unsigned char type;

	// ���� �� ����� room manager�� ������ �̵�
};

struct sc_packet_chat {
	unsigned char	size;
	char			type;
	int				id;
	char			message[MAX_CHAT_SIZE];
};