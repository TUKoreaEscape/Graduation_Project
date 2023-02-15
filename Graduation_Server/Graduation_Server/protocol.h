#pragma once

// 클라이언트와 서버간 통신에 사용할 구조체를 정의합니다.
const short SERVER_PORT = 4000;
const int	BUFSIZE = 256;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 30;
// ----- 클라이언트가 서버에게 보낼때 ------


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
		CS_CREATE_ID,
		CS_LOGIN,
		CS_MOVE,
		CS_PACKET_CHAT,
		CS_PACKET_CREATE_ROOM,
		CS_PACKET_JOIN_ROOM
	};
}

struct cs_packet_create_id {
	unsigned char	size;
	unsigned char	type;

	char			id[20];
	char			pass_word[20];
};

struct cs_packet_login { // 로그인 시도
	unsigned char	size;
	unsigned char	type;

	char			id[20];
	char			pass_word[20];
};

struct cs_packet_move { // 이동관련 데이터
	unsigned char	size;
	unsigned char	type;

	unsigned char	input_key;
	unsigned char	state;

	unsigned short	look;

	// 벡터를 보낼지도 고민중
};

struct cs_packet_voice {
	unsigned char size;
	unsigned char type;

	// vivox사용 예정
};

struct cs_packet_chat {
	unsigned char	size;
	int				room_number;
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
		SC_CREATE_ID_OK,
		SC_CREATE_ID_FAIL,
		SC_MOVING,
		SC_PACKET_CHAT
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
	
	unsigned short	look;
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