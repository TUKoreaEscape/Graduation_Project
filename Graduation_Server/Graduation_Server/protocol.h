#pragma once

// 클라이언트와 서버간 통신에 사용할 구조체를 정의합니다.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 256;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 20;
const int  MAX_ROOM = 5000;
// ----- 클라이언트가 서버에게 보낼때 ------



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

struct Roominfo_by10 {
	int						room_number;
	char					room_name[MAX_NAME_SIZE];

	GAME_ROOM_STATE::TYPE	state;
	int						join_member;
};


struct cs_packet_create_id {
	unsigned char	size;
	unsigned char	type;

	char			id[MAX_NAME_SIZE];
	char			pass_word[MAX_NAME_SIZE];
};

struct cs_packet_login { // 로그인 시도
	unsigned char	size;
	unsigned char	type;

	char			id[MAX_NAME_SIZE];
	char			pass_word[MAX_NAME_SIZE];
};

struct cs_packet_move { // 이동관련 데이터
	unsigned char	size;
	unsigned char	type;

	unsigned char	input_key;
	unsigned char	state;

	Position		pos;
};

struct cs_packet_voice {
	unsigned char size;
	unsigned char type;

	// vivox사용 예정
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

struct cs_packet_request_all_room_info {
	unsigned char	size;
	unsigned char	type;

	int				request_page;
};

// ----- 서버가 클라이언트에게 보낼때 -----

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
		SC_PACKET_ROOM_INFO
	};
}

struct sc_packet_login_ok { // 로그인 성공을 클라에게 전송
	unsigned char	size;
	unsigned char	type;
	unsigned int	id;
};

struct sc_packet_login_fail { // 로그인 실패를 클라에게 전송
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

	unsigned int	room_count;
	Roominfo_by10	room_info[10];
};

struct sc_packet_create_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// 방 생성 내용
};

struct sc_packet_user_join_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// 유저 방 입장시 room manager에 데이터 이동
};

struct sc_packet_chat { // 유저간 채팅
	unsigned char	size;
	unsigned char	type;
	
	int				id;
	
	char			message[MAX_CHAT_SIZE];
};