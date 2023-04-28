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
	packet.type = SC_PACKET::SC_PACKET_LOGINFAIL;
	packet.size = sizeof(sc_packet_login_fail);
	packet.reason = reason;
	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_login_ok_packet(const unsigned int user_id)
{
	sc_packet_login_ok packet;
	packet.id = user_id;
	packet.size = sizeof(sc_packet_login_ok);
	packet.type = SC_PACKET::SC_PACKET_LOGINOK;

	// 여긴 커마정보 로드된 이후에 전송됨
	packet.body = m_clients[user_id].m_customizing->Get_Body_Custom();
	packet.body_parts = m_clients[user_id].m_customizing->Get_Body_Part_Custom();
	packet.eyes = m_clients[user_id].m_customizing->Get_Eyes_Custom();
	packet.gloves = m_clients[user_id].m_customizing->Get_Gloves_Custom();
	packet.head = m_clients[user_id].m_customizing->Get_Head_Custom();
	packet.mouthandnoses = m_clients[user_id].m_customizing->Get_Mouthandnoses_Custom();

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_ok_packet(const unsigned int user_id)
{
	sc_packet_create_id_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CREATE_ID_OK;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_fail_packet(const unsigned int user_id, char reason)
{
	sc_packet_create_id_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CREATE_ID_FAIL;
	packet.reason = reason;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_room_ok_packet(const unsigned int user_id, const int room_number)
{
	sc_packet_create_room packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CREATE_ROOM_OK;
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

void cGameServer::send_move_packet(const unsigned int id, const unsigned int moved_id, cs_packet_move recv_packet, XMFLOAT3 calculate_pos)
{
	sc_packet_move packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_MOVE;
	packet.pos.x = static_cast<short>(calculate_pos.x * 100);
	packet.pos.y = static_cast<short>(calculate_pos.y * 100);
	packet.pos.z = static_cast<short>(calculate_pos.z * 100);
	packet.id = moved_id;
	packet.input_key = recv_packet.input_key;
	packet.look = recv_packet.look;
	packet.right = recv_packet.right;

	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_move_packet(const unsigned int id, const unsigned int moved_id)
{
	sc_packet_move packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_MOVE;
	m_clients[moved_id]._update_lock.lock();
	packet.pos.x = static_cast<short>(m_clients[moved_id].get_user_position().x * 100);
	packet.pos.y = static_cast<short>(m_clients[moved_id].get_user_position().y * 100);
	packet.pos.z = static_cast<short>(m_clients[moved_id].get_user_position().z * 100);
	packet.id = moved_id;
	packet.input_key = m_clients[moved_id].get_input_key();

	packet.look.x = m_clients[moved_id].get_look_x();
	packet.look.y = m_clients[moved_id].get_look_y();
	packet.look.z = m_clients[moved_id].get_look_z();

	packet.right.x = m_clients[moved_id].get_right_x();
	packet.right.y = m_clients[moved_id].get_right_y();
	packet.right.z = m_clients[moved_id].get_right_z();
	m_clients[moved_id]._update_lock.unlock();
	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_calculate_move_packet(const unsigned int id) // 이동을 요청한 클라이언트 이동정보를 계산하여 좌표를 전해줌
{
	sc_packet_calculate_move packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CALCULATE_MOVE;
	packet.id = id;
	packet.pos.x = static_cast<short>(m_clients[id].get_user_position().x * 100);
	packet.pos.y = static_cast<short>(m_clients[id].get_user_position().y * 100);
	packet.pos.z = static_cast<short>(m_clients[id].get_user_position().z * 100);
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
	sc_put_player_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_PLAYER;
	packet.data.active = false;
	packet.data.id = recv_id;
	packet.data.position = XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	packet.data.velocity = XMFLOAT3{ 0,0,0 };
	packet.data.yaw = 0.0f;

	m_clients[recv_id].set_user_position(XMFLOAT3(0, 0, 0)); // 이건 임시사용입니다.
	m_clients[recv_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_put_other_player(const unsigned int put_id, const unsigned int recv_id)
{
	sc_put_player_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER;
	packet.data.active = false;
	packet.data.id = put_id;
	packet.data.position = m_clients[put_id].get_user_position();
#if PRINT
	cout << "유저 정보를 보넀습니다 : " << put_id << "->" << recv_id << endl;
#endif
	m_clients[recv_id].do_send(sizeof(packet), &packet);

	sc_packet_customizing_update customizing_packet;
	customizing_packet.id = put_id;
	customizing_packet.size = sizeof(customizing_packet);
	customizing_packet.type = SC_PACKET::SC_PACKET_CUSTOMIZING;

	customizing_packet.body = static_cast<int>(m_clients[put_id].m_customizing->Get_Body_Custom());
	customizing_packet.body_parts = static_cast<int>(m_clients[put_id].m_customizing->Get_Body_Part_Custom());
	customizing_packet.eyes = static_cast<int>(m_clients[put_id].m_customizing->Get_Eyes_Custom());
	customizing_packet.gloves = static_cast<int>(m_clients[put_id].m_customizing->Get_Gloves_Custom());
	customizing_packet.head = static_cast<int>(m_clients[put_id].m_customizing->Get_Head_Custom());
	customizing_packet.mouthandnoses = static_cast<int>(m_clients[put_id].m_customizing->Get_Mouthandnoses_Custom());

	m_clients[recv_id].do_send(sizeof(customizing_packet), &customizing_packet);
}

void cGameServer::send_customizing_data(const unsigned int id)
{
	sc_packet_customizing_update packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CUSTOMIZING;
	packet.id = id;
	packet.body = static_cast<int>(m_clients[id].m_customizing->Get_Body_Custom());
	packet.body_parts = static_cast<int>(m_clients[id].m_customizing->Get_Body_Part_Custom());
	packet.eyes = static_cast<int>(m_clients[id].m_customizing->Get_Eyes_Custom());
	packet.gloves = static_cast<int>(m_clients[id].m_customizing->Get_Gloves_Custom());
	packet.head = static_cast<int>(m_clients[id].m_customizing->Get_Head_Custom());
	packet.mouthandnoses = static_cast<int>(m_clients[id].m_customizing->Get_Mouthandnoses_Custom());

	m_clients[id]._room_list_lock.lock();
	for (auto p : m_clients[id].room_list)
	{
		m_clients[p].do_send(sizeof(packet), &packet);
	}
	m_clients[id]._room_list_lock.unlock();
}