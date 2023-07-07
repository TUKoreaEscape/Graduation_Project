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

void cGameServer::send_move_packet(const unsigned int id, const unsigned int moved_id)
{
	sc_packet_move move_packet;
	move_packet.size = sizeof(move_packet);
	move_packet.type = SC_PACKET::SC_PACKET_MOVE;
	move_packet.data.id = moved_id;
	move_packet.data.input_key = m_clients[moved_id].get_input_key();
	move_packet.data.look.x = m_clients[moved_id].get_look_x();
	move_packet.data.look.y = m_clients[moved_id].get_look_y();
	move_packet.data.look.z = m_clients[moved_id].get_look_z();
	move_packet.data.right.x = m_clients[moved_id].get_right_x();
	move_packet.data.right.y = m_clients[moved_id].get_right_y();
	move_packet.data.right.z = m_clients[moved_id].get_right_z();
	move_packet.data.position.x = static_cast<int>(m_clients[moved_id].get_user_position().x * 10000);
	move_packet.data.position.y = static_cast<int>(m_clients[moved_id].get_user_position().y * 10000);
	move_packet.data.position.z = static_cast<int>(m_clients[moved_id].get_user_position().z * 10000);
	move_packet.data.is_jump = m_clients[moved_id].get_user_is_jump();
	move_packet.data.is_attack = m_clients[moved_id].get_user_attack_animation();
	move_packet.data.is_victim = m_clients[moved_id].get_user_victim_animation();
	move_packet.data.is_collision_up_face = m_clients[moved_id].get_user_collied_up_face();
	move_packet.data.active = m_clients[moved_id].get_life_chip();

	m_clients[id].do_send(sizeof(move_packet), &move_packet);
}

void cGameServer::send_calculate_move_packet(const unsigned int id) // 이동을 요청한 클라이언트 이동정보를 계산하여 좌표를 전해줌
{
	sc_packet_calculate_move packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CALCULATE_MOVE;
	packet.id = id;
	packet.pos.x = static_cast<int>(m_clients[id].get_user_position().x * 10000);
	packet.pos.y = static_cast<int>(m_clients[id].get_user_position().y * 10000);
	packet.pos.z = static_cast<int>(m_clients[id].get_user_position().z * 10000);
	packet.is_collision_up_face = m_clients[id].get_user_collied_up_face();

	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_life_chip_update(const unsigned int id) // 생명칩 일괄 전송!
{
	sc_packet_life_chip_update packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_LIFE_CHIP_UPDATE;
	packet.id = id;
	packet.life_chip = m_clients[id].get_life_chip();

	m_clients[id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_correct_life_chip(const unsigned int id)
{
	Room& room = *m_room_manager->Get_Room_Info(m_clients[id].get_join_room_number());

	// 이쪽으로 넘어온 경우 술래가 다른 플레이어 공격에 성공한것임
	// 즉 술래에게 지금 수집한 생명칩이 있음을 전달해야함
	// 이후 술래가 재단에 가서 생명칩을 보관하면 방에 수집한 생명칩 갯수를 업데이트, 일정 갯수를 채운 경우 게임 종료 처리 해야함.
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
	Room& rl = *m_room_manager->Get_Room_Info(m_clients[recv_id].get_join_room_number());
	int when_join = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (rl.Get_Join_Member(i) == recv_id)
			when_join = i;
	}

	m_clients[recv_id].set_user_position(XMFLOAT3(static_cast<float>(6.f - ((float)when_join * 2.5)), 5.f, -4.f));
	sc_put_player_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_PLAYER;
	packet.data.active = false;
	packet.data.id = recv_id;
	//packet.data.position = m_clients[recv_id].get_user_position();
	packet.data.velocity = XMFLOAT3{ 0,0,0 };
	packet.data.yaw = 0.0f;
	m_clients[recv_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_put_other_player(const unsigned int put_id, const unsigned int recv_id)
{
	sc_put_player_packet packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER;
	packet.data.active = false;
	packet.data.id = put_id;
	packet.is_ready = m_clients[put_id].m_is_ready;
	//packet.data.position = m_clients[put_id].get_user_position();
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