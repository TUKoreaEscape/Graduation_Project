#pragma once
#include <DirectXMath.h>
// Ŭ���̾�Ʈ�� ������ ��ſ� ����� ����ü�� �����մϴ�.
const short SERVER_PORT = 5000;
const int	BUFSIZE = 1024;
const int	MAX_CHAT_SIZE = 100;

const int  MAX_NAME_SIZE = 20;
const int  MAX_ROOM = 5000;

const int  MAX_ROOM_INFO_SEND = 6;

const int CHECK_MAX_PACKET_SIZE = 127;

const int BUF_SIZE = 1024;
// ----- Ŭ���̾�Ʈ�� �������� ������ ------

#define VOICE_ISSUER "�ۼ��ؾߵ�"
#define VOICE_DOMAIN "�ۼ��ؾߵ�"
#define VOICE_KEY "�ۼ��ؾߵ�"
#define VOICE_API "�ۼ��ؾߵ�"

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
		CS_PACKET_USE_THIRD_TAGGER_SKILL,
		CS_PACKET_REQUEST_OPEN_DOOR,
		CS_PACKET_REQUEST_OPEN_HIDDEN_DOOR,
		CS_PACKET_REQUEST_ELETRONIC_SYSTEM_DOOR,
		CS_PACKET_REQUEST_ELETRONIC_SYSTEM_SWICH,
		CS_PACKET_ATTACK,
		CS_PACKET_PICK_ITEM,
		CS_PACKET_STRESS_LOGIN
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

struct ElectronicSystem_Data {
	unsigned short	idx;
	unsigned char	value[15];
};

struct UserData {
	short				id;
	unsigned char		input_key;
	Position			position;
	Look				look;
	Right				right;
	bool				is_jump;
	bool				is_collision_up_face;
	bool				is_attack;
	bool				is_victim;
	unsigned char		active; // ����Ĩ���� :D
};

struct PutData {
	short				id;
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT3	velocity;
	float				yaw; // ����
	unsigned char		active; // ����Ĩ���� :D
};


struct Roominfo_by10 {
	unsigned short				room_number;
	char						room_name[10];
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

	int				room_number;
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

struct cs_packet_attack {
	unsigned char	size;
	unsigned char	type;
};

struct cs_packet_use_tagger_skill {
	unsigned char	size;
	unsigned char	type;
};

struct cs_packet_request_open_door {
	unsigned char	size;
	unsigned char	type;

	unsigned char   door_num;
};

struct cs_packet_request_open_hidden_door {
	unsigned char	size;
	unsigned char	type;
};

struct cs_packet_request_electronic_system_open {
	unsigned char	size;
	unsigned char	type;
	bool			is_door_open;
	unsigned short	es_num;
};

struct cs_packet_request_eletronic_system_switch_control {
	unsigned char	size;
	unsigned char	type;

	unsigned short	electronic_system_index;
	unsigned short	switch_idx;
	bool			switch_value;
};

struct cs_packet_request_electronic_system_fix {
	unsigned char	size;
	unsigned char	type;

	unsigned char	fix_item_info;
};

struct cs_packet_pick_fix_item {
	unsigned char	size;
	unsigned char	type;

	unsigned short	item_type;
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

// ----- ������ Ŭ���̾�Ʈ���� ������ -----

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
		SC_PACKET_ROOM_INFO_UPDATE,
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
		SC_PACKET_ELECTRONIC_SWITCH_INIT,
		SC_PACKET_CALCULATE_MOVE,
		SC_PACKET_SELECT_TAGGER,
		SC_PACKET_TAGGER_SKILL,
		SC_PACKET_DOOR_UPDATE,
		SC_PACKET_ELECTRONIC_SYSTEM_DOOR_UPDATE,
		SC_PACKET_ELECTRONIC_SYSTEM_SWITCH_UPDATE,
		SC_PACKET_ROOM_INFO,
		SC_PACKET_VIVOX_DATA,
		SC_PACKET_CUSTOMIZING,
		SC_PACKET_ATTACK,
		SC_PACKET_GAME_END
	};
}

struct sc_packet_login_ok { // �α��� ������ Ŭ�󿡰� ���� + Ŀ�������� �ϰ� ����
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

struct sc_packet_login_fail { // �α��� ���и� Ŭ�󿡰� ����
	unsigned char	size;
	unsigned char	type;
	unsigned char   reason;
};

struct sc_packet_create_id_ok { // ���̵� ���� ������ ��������
	unsigned char	size;
	unsigned char	type;
};

struct sc_packet_create_id_fail { // ���̵� ���� ���и� �������� (�����ڵ�� �Բ�)
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

struct sc_packet_game_start { // ���� ������ �濡 �ִ� �÷��̾�� �˷���
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

	short			id;

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

struct sc_packet_electronic_system_init {
	unsigned char	size;
	unsigned char	type;

	ElectronicSystem_Data data[5];
};

struct sc_packet_attack {
	unsigned char	size;
	unsigned char	type;
	short			attacker_id;
	short			victim_id;
};

struct sc_packet_open_door {
	unsigned char	size;
	unsigned char	type;

	unsigned char	door_number;
	unsigned char	door_state;
};

struct sc_packet_open_electronic_system_door {
	unsigned char	size;
	unsigned char	type;

	unsigned short	es_num;
	unsigned char	es_state;
};

struct sc_packet_electronic_system_update_value {
	unsigned char	size;
	unsigned char	type;

	unsigned short	es_num;
	unsigned short	es_switch_idx;
	bool			es_value;
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

struct sc_packet_update_room {
	unsigned char	size;
	unsigned char	type;

	short			room_number;
	short			join_member;
	GAME_ROOM_STATE::TYPE state;
};

struct sc_packet_game_end {
	unsigned char	size;
	unsigned char	type;

	bool			is_tagger_win;
};
#pragma pack(pop)