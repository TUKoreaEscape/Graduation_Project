#pragma once

// Ŭ���̾�Ʈ�� ������ ��ſ� ����� ����ü�� �����մϴ�.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 1024;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 20;
const int  MAX_ROOM = 5000;

const int  MAX_ROOM_INFO_SEND = 10;
// ----- Ŭ���̾�Ʈ�� �������� ������ ------



namespace GAME_ROOM_STATE
{
	enum TYPE
	{
		NONE = 0,
		FREE,
		READY,
		PLAYING
	};
}

namespace LOGIN_FAIL_REASON
{
	enum TYPE
	{
		INVALID_ID,
		WRONG_PW
	};
}

namespace CS_PACKET
{
	enum TYPE
	{
		NONE = 0,
		CS_CREATE_ID,
		CS_LOGIN,
		CS_MOVE,
		CS_PACKET_CHAT,
		CS_PACKET_CREATE_ROOM,
		CS_PACKET_JOIN_ROOM,
		CS_PACKET_EXIT_ROOM,
		CS_PACKET_REQUEST_ROOM_INFO
	};
}

struct Position {
	float x;
	float y;
	float z;
	float look;
	float at;
};
//#pragma pack (push, 1)
struct Roominfo_by10 {
	unsigned short				room_number;
	char						room_name[20];
	unsigned short				join_member;
	GAME_ROOM_STATE::TYPE		state;
};


struct cs_packet_create_id {
	unsigned char	size;
	unsigned char	type;

	char			id[MAX_NAME_SIZE];
	char			pass_word[MAX_NAME_SIZE];
};

struct cs_packet_login { // �α��� �õ�
	unsigned char	size;
	unsigned char	type;

	char			id[MAX_NAME_SIZE];
	char			pass_word[MAX_NAME_SIZE];
};

struct cs_packet_move { // �̵����� ������
	unsigned char	size;
	unsigned char	type;

	unsigned char	input_key;
	unsigned char	state;

	Position		pos;
};

struct cs_packet_voice {
	unsigned char size;
	unsigned char type;

	// vivox��� ����
};

struct cs_packet_chat {
	unsigned char	size;
	unsigned char	type;

	int				room_number;
	char			message[MAX_CHAT_SIZE];
};

struct cs_packet_create_room {
	unsigned char	size;
	unsigned char	type;
};

struct cs_packet_join_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
};

struct cs_packet_request_exit_room {
	unsigned char	size;
	unsigned char	type;

	unsigned char	request_page;
};

struct cs_packet_request_all_room_info {
	unsigned char	size;
	unsigned char	type;

	unsigned char   request_page;
};

// ----- ������ Ŭ���̾�Ʈ���� ������ -----

namespace SC_PACKET
{
	enum TYPE
	{
		NONE = 0,
		SC_LOGINOK,
		SC_LOGINFAIL,
		SC_CREATE_ID_OK,
		SC_CREATE_ID_FAIL,
		SC_CREATE_ROOM_OK,
		SC_MOVING,
		SC_PACKET_CHAT,
		SC_PACKET_JOIN_ROOM_SUCCESS,
		SC_PACKET_JOIN_ROOM_FAIL,
		SC_PACKET_ROOM_INFO
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
	unsigned char   reason;
};

struct sc_packet_create_id_ok {
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_create_id_fail {
	unsigned char	size;
	unsigned char	type;
	unsigned char	reason;
};

struct sc_packet_move {
	unsigned char	size;
	unsigned char	type;

	unsigned char	input_key;
	unsigned char	state;

	Position		pos;
};

struct sc_packet_request_room_info {
	unsigned char	size;
	unsigned char	type;

	Roominfo_by10	room_info[MAX_ROOM_INFO_SEND];
};

struct sc_packet_create_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// �� ���� ����
};

struct sc_packet_join_room_success {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// ���� �� ����� room manager�� ������ �̵�
};

struct sc_packet_join_room_fail {
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_chat { // ������ ä��
	unsigned char	size;
	unsigned char	type;

	unsigned int	id;

	unsigned char	message[MAX_CHAT_SIZE];
};

//#pragma pack(pop)