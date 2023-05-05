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

const int BUF_SIZE = 1024;
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
		CS_PACKET_CREATE_ID,
		CS_PACKET_LOGIN,
		CS_PACKET_MOVE,
		CS_PACKET_CHAT,
		CS_PACKET_CREATE_ROOM,
		CS_PACKET_JOIN_ROOM,
		CS_PACKET_EXIT_ROOM,
		CS_PACKET_READY,
		CS_PACKET_GAME_LOADING_SUCCESS,
		CS_PACKET_REQUEST_ROOM_INFO,
		CS_PACKET_REQUEST_VIVOX_DATA,
		CS_PACKET_CUSTOMIZING,
		CS_PACKET_USE_FIRST_TAGGER_SKILL,
		CS_PACKET_USE_SECOND_TAGGER_SKILL,
		CS_PACKET_USE_THIRD_TAGGER_SKILL
	};
}

#pragma pack (push, 1)
struct Position {
	int x;
	int y;
	int z;
};

struct Look {
	short x;
	short y;
	short z;
};

struct Right {
	short x;
	short y;
	short z;
};

struct UserData {
	short				id;
	unsigned char		input_key;
	Position			position;
	Look				look;
	Right				right;
	bool				is_jump;
	bool				is_collision_up_face;
	unsigned char		active; // 생명칩유무 :D
};

struct PutData {
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
	unsigned char		size;
	unsigned char		type;

	unsigned char		input_key;
	bool				is_jump;
	float				yaw;
	Look				look;
	Right				right;
	DirectX::XMFLOAT3	velocity;
	DirectX::XMFLOAT3	xmf3Shift;
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

struct cs_packet_use_tagger_skill {
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

struct cs_packet_customizing_update {
	unsigned char	size;
	unsigned char	type;

	unsigned char	head;
	unsigned char	body;
	unsigned char	body_parts;
	unsigned char	eyes;
	unsigned char	gloves;
	unsigned char	mouthandnoses;
};

// ----- 서버가 클라이언트에게 보낼때 -----

namespace SC_PACKET
{
	enum TYPE
	{
		NONE = 0,
		SC_PACKET_LOGINOK,
		SC_PACKET_LOGINFAIL,
		SC_PACKET_CREATE_ID_OK,
		SC_PACKET_CREATE_ID_FAIL,
		SC_PACKET_CREATE_ROOM_OK,
		SC_PACKET_USER_UPDATE,
		SC_PACKET_OTHER_PLAYER_UPDATE,
		SC_PACKET_OTHER_PLAYER_DISCONNECT,
		SC_PACKET_CHAT,
		SC_PACKET_JOIN_ROOM_SUCCESS,
		SC_PACKET_JOIN_ROOM_FAIL,
		SC_PACKET_GAME_START,
		SC_PACKET_PUT_PLAYER,
		SC_PACKET_PUT_OTHER_PLAYER,
		SC_PACKET_MOVE,
		SC_PACKET_CALCULATE_MOVE,
		SC_PACKET_SELECT_TAGGER,
		SC_PACKET_TAGGER_SKILL,
		SC_PACKET_ROOM_INFO,
		SC_PACKET_VIVOX_DATA,
		SC_PACKET_CUSTOMIZING
	};
}

struct sc_packet_login_ok { // 로그인 성공을 클라에게 전송 + 커마정보도 일괄 전송
	unsigned char	size;
	unsigned char	type;
	unsigned int	id;

	unsigned char	head;
	unsigned char	head_parts;
	unsigned char	body;
	unsigned char	body_parts;
	unsigned char	eyes;
	unsigned char	gloves;
	unsigned char	mouthandnoses;
	unsigned char	tails;
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
	unsigned char	sub_size_mul;
	unsigned char	sub_size_add;
	UserData		data[6];
};

struct sc_put_player_packet {
	unsigned char	size;
	unsigned char	type;

	PutData		data;
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

	unsigned char	input_key;

	Look		look;
	Right		right;
	Position	pos;
};

struct sc_packet_calculate_move {
	unsigned char	size;
	unsigned char	type;
	short			id;
	Position		pos;
	bool			is_collision_up_face;
};

struct sc_packet_select_tagger {
	unsigned char	size;
	unsigned char	type;

	bool			first_skill;
	bool			second_skill;
	bool			third_skill;
};

struct sc_packet_tagger_skill {
	unsigned char	size;
	unsigned char	type;

	bool			first_skill;
	bool			second_skill;
	bool			third_skill;
};


struct sc_other_player_move {
	unsigned char	size;
	unsigned char	type;
	UserData		data[5];
};

struct sc_other_player_disconnect {
	unsigned char	size;
	unsigned char	type;
	short			id;
};

struct sc_packet_customizing_update {
	unsigned char	size;
	unsigned char	type;
	short			id;

	unsigned char	head;
	unsigned char	body;
	unsigned char	body_parts;
	unsigned char	eyes;
	unsigned char	gloves;
	unsigned char	mouthandnoses;
};
#pragma pack(pop)
