#include "GameServer.h"

void cGameServer::send_chat_packet(const unsigned int user_id, const unsigned int my_id, char* mess)
{
	sc_packet_chat packet;
	//packet.id = my_id;
	m_clients[my_id].get_client_name(*packet.name, MAX_NAME_SIZE);
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CHAT;
	strcpy_s(packet.message, mess);

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_login_fail_packet(const unsigned int user_id, LOGIN_FAIL_REASON::TYPE reason)
{
	sc_packet_login_fail packet;
	packet.type = SC_PACKET::SC_LOGINFAIL;
	packet.size = sizeof(sc_packet_login_fail);
	packet.reason = reason;
	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_login_ok_packet(const unsigned int user_id)
{
	sc_packet_login_ok packet;
	packet.id = user_id;
	packet.size = sizeof(sc_packet_login_ok);
	packet.type = SC_PACKET::SC_LOGINOK;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_ok_packet(const unsigned int user_id)
{
	sc_packet_create_id_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ID_OK;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_fail_packet(const unsigned int user_id, char reason)
{
	sc_packet_create_id_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ID_FAIL;
	packet.reason = reason;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_room_ok_packet(const unsigned int user_id, const int room_number)
{
	sc_packet_create_room packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ROOM_OK;
	packet.room_number = room_number;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_join_room_success_packet(const unsigned int user_id)
{
	sc_packet_join_room_success packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_JOIN_ROOM_SUCCESS;

	packet.room_number = m_clients[user_id].get_join_room_number();

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_join_room_fail_packet(const unsigned int user_id)
{
	sc_packet_join_room_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_game_start_packet(const unsigned int id)
{
	sc_packet_game_start packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_GAME_START;

	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_move_packet(const unsigned int id, const unsigned int moved_id, XMFLOAT3 pos)
{
	sc_update_user_packet packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_MOVE;

	packet.data.id = moved_id;
	packet.data.position = pos;
	packet.data.velocity = m_clients[moved_id].get_user_velocity();
	packet.data.yaw = m_clients[moved_id].get_user_yaw();
	packet.data.active;
	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_voice_data(const unsigned int id)
{
	sc_packet_voice_data packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_VIVOX_DATA;

	strcpy_s(packet.api, strlen(VOICE_API), VOICE_API);
	strcpy_s(packet.domain, strlen(VOICE_DOMAIN), VOICE_DOMAIN);
	strcpy_s(packet.issuer, strlen(VOICE_ISSUER), VOICE_ISSUER);
	strcpy_s(packet.key, strlen(VOICE_KEY), VOICE_KEY);

	packet.join_room_number = m_clients[id].get_join_room_number();

	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_put_player_data(const unsigned int recv_id)
{
	sc_update_user_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_PLAYER;
	packet.data.active = false;
	packet.data.id = recv_id;
	packet.data.position = XMFLOAT3{ recv_id * 20.0f, 0.0f, 0.0f };
	packet.data.velocity = XMFLOAT3{ 0,0,0 };
	packet.data.yaw = 0.0f;

	m_clients[recv_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_put_other_player(const unsigned int put_id, const unsigned int recv_id)
{
	sc_update_user_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER;
	packet.data.active = false;
	packet.data.id = put_id;
	packet.data.position = XMFLOAT3{ put_id * 1.0f, 0.0f, 0.0f };
	packet.data.velocity = XMFLOAT3{ 0,0,0 };
	packet.data.yaw = 0.0f;
#if PRINT
	cout << "유저 정보를 보넀습니다." << endl;
#endif
	m_clients[recv_id].do_send(sizeof(packet), &packet);
}