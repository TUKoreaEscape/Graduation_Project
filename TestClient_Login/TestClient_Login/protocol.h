#pragma once

// Ŭ���̾�Ʈ�� ������ ��ſ� ����� ����ü�� �����մϴ�.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 256;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 30;
// ----- Ŭ���̾�Ʈ�� �������� ������ ------
enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT, OP_NPC_MOVE, OP_PLAYER_MOVE };

namespace GAME_ROOM_STATE
{
	enum TYPE
	{
		NONE = 0,
		READY,
		PLAYING
	};
}

namespace CS_PACKET
{
	enum TYPE
	{
		NONE = 0,
		CS_LOGIN,
		CS_MOVE,
		CS_PACKET_CHAT,
		CS_PACKET_CREATE_ROOM
	};
}


struct cs_packet_login { // �α��� �õ�
	unsigned char	size;
	unsigned char	type;

	char			id[20];
	char			pass_word[20];
};

struct cs_packet_move { // �̵����� ������
	unsigned char	size;
	unsigned char	type;

	unsigned char	input_key;
	// ���͸� �������� �����
};

struct cs_packet_voice {
	unsigned char size;
	unsigned char type;

	// vivox��� ����
};

struct cs_packet_chat {
	unsigned char	size;
	int				room_number;
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

struct sc_packet_login_ok { // �α��� ������ Ŭ�󿡰� ����
	unsigned char	size;
	unsigned char	type;
	unsigned int	id;
};

struct sc_packet_login_fail { // �α��� ���и� Ŭ�󿡰� ����
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_move {
	unsigned char	size;
	unsigned char	type;

	char			input_key;
};

struct sc_packet_create_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// �� ���� ����
};

struct sc_packet_user_join_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// ���� �� ����� room manager�� ������ �̵�
};

struct sc_packet_chat { // ������ ä��
	unsigned char	size;
	char			type;
	int				id;
	char			message[MAX_CHAT_SIZE];
};