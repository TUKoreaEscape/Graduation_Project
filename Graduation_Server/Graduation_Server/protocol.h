#pragma once
#include <DirectXMath.h>
// 클라이언트와 서버간 통신에 사용할 구조체를 정의합니다.
const short SERVER_PORT = 5000;
const int	BUFSIZE = 1024;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 20;
const int  MAX_ROOM = 5000;

const int  MAX_ROOM_INFO_SEND = 10;

const int CHECK_MAX_PACKET_SIZE = 127;
// ----- 클라이언트가 서버에게 보낼때 ------

#define VOICE_ISSUER "작성해야됨"
#define VOICE_DOMAIN "작성해야됨"
#define VOICE_KEY "작성해야됨"
#define VOICE_API "작성해야됨"

namespace GAME_ROOM_STATE
{
	enum TYPE
	{
		NONE = 0,
		FREE,
		READY,
		PLAYING,
		END
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
		CS_PACKET_READY,
		CS_PACKET_GAME_LOADING_SUCCESS,
		CS_PACKET_REQUEST_ROOM_INFO,
		CS_PACKET_REQUEST_VIVOX_DATA
	};
}

#pragma pack (push, 1)
struct UserData {
	short				id;
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT3	velocity;
	float				yaw; // 각도
	unsigned char		active; // 생명칩유무 :D
};


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

struct cs_packet_login { // 로그인 시도
	unsigned char	size;
	unsigned char	type;

	char			id[MAX_NAME_SIZE];
	char			pass_word[MAX_NAME_SIZE];
};

struct cs_packet_move { // 이동관련 데이터
	unsigned char	size;
	unsigned char	type;

	DirectX::XMFLOAT3	position;
	float				yaw;
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

struct cs_packet_join_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
};

struct cs_packet_ready {
	unsigned char	size;
	unsigned char	type;

	bool			ready_type;
};

struct cs_packet_loading_success {
	unsigned char	size;
	unsigned char	type;
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

struct cs_packet_request_vivox_data {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
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
		SC_USER_UPDATE,
		SC_PACKET_CHAT,
		SC_PACKET_JOIN_ROOM_SUCCESS,
		SC_PACKET_JOIN_ROOM_FAIL,
		SC_PACKET_GAME_START,
		SC_PACKET_PUT_PLAYER,
		SC_PACKET_PUT_OTHER_PLAYER,
		SC_PACKET_MOVE,
		SC_PACKET_ROOM_INFO,
		SC_PACKET_VIVOX_DATA
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

struct sc_packet_create_id_ok { // 아이디 생성 성공을 전송해줌
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_create_id_fail { // 아이디 생성 실패를 전송해줌 (사유코드와 함께)
	unsigned char	size;
	unsigned char	type;
	unsigned char	reason;
};

struct sc_update_user_packet {
	unsigned char	size;
	unsigned char	type;

	UserData		data;
};

struct sc_packet_request_room_info {
	unsigned char	size;
	unsigned char	type;
	unsigned char	sub_size_mul;
	unsigned char	sub_size_add;

	Roominfo_by10	room_info[MAX_ROOM_INFO_SEND];
};

struct sc_packet_create_room {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// 방 생성 내용
};

struct sc_packet_join_room_success {
	unsigned char	size;
	unsigned char	type;

	unsigned int	room_number;
	// 유저 방 입장시 room manager에 데이터 이동
};

struct sc_packet_join_room_fail {
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_chat { // 유저간 채팅
	unsigned char	size;
	unsigned char	type;

	char			name[MAX_NAME_SIZE];
	char			message[MAX_CHAT_SIZE];
};

struct sc_packet_put_other_client {
	unsigned char		size;
	unsigned char		type;
	unsigned int		user_id;

	char				id[MAX_NAME_SIZE];
	DirectX::XMFLOAT3	position;
	float				yaw;
};

struct sc_packet_game_start { // 게임 시작을 방에 있는 플레이어에게 알려줌
	unsigned char	size;
	unsigned char	type;
};



struct sc_packet_voice_data {
	unsigned char	size;
	unsigned char	type;

	unsigned int	join_room_number;

	char			issuer[20];
	char			domain[15];
	char			key[8];
	char			api[33];
};

struct sc_packet_move {
	unsigned char	size;
	unsigned char	type;
	unsigned short	id;

	DirectX::XMFLOAT3	pos;
	float				yaw;
};
#pragma pack(pop)
