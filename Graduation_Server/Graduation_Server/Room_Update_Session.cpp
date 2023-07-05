#include "Room.h"
#include "GameServer.h"

int Room::Select_Tagger()
{
	mt19937 engine((unsigned int)time(NULL));
	int tagger_id = engine() % 6;
	m_tagger_id = in_player[tagger_id];

	cGameServer& server = *cGameServer::GetInstance();

	for (auto player_id : in_player) {
		if (player_id == m_tagger_id)
			continue;
		server.m_clients[player_id].set_life_chip(true);
	}
	return tagger_id;
}

void Room::Start_Game()
{
	start_time = chrono::system_clock::now();
	_room_state_lock.lock();
	_room_state = GAME_ROOM_STATE::PLAYING;
	_room_state_lock.unlock();
	cGameServer& server = *cGameServer::GetInstance();

	for (auto player_id : in_player) {
		if (player_id == -1)
			continue;
		server.m_clients[player_id].set_life_chip(false);
	}

	sc_packet_update_room update_room_packet;
	update_room_packet.size = sizeof(update_room_packet);
	update_room_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE;
	update_room_packet.join_member = Get_Number_of_users();
	update_room_packet.room_number = room_number;
	_room_state_lock.lock();
	update_room_packet.state = _room_state;
	_room_state_lock.unlock();

	for (auto& cl : server.m_clients)
	{
		if (cl.get_state() != CLIENT_STATE::ST_LOBBY)
			continue;

		if (cl.get_look_lobby_page() != room_number / 6)
			continue;

		cl.do_send(sizeof(update_room_packet), &update_room_packet);
	}

	auto start_time = chrono::system_clock::now();
	TIMER_EVENT ev;
	ev.room_number = room_number;
	ev.event_type = EventType::GAME_END;
	ev.event_time = start_time + static_cast<chrono::seconds>(GAME_END_SECOND);
	server.m_timer_queue.push(ev);

	ev.event_type = EventType::SELECT_TAGGER;
	ev.event_time = start_time + 10s;
	server.m_timer_queue.push(ev);

	Set_Electronic_System_ONOFF();

	// 여기서 전력장치 스위치 정보 다 보내줘야함
	sc_packet_electronic_system_init system_packet;
	system_packet.size = sizeof(system_packet);
	system_packet.type = SC_PACKET::SC_PACKET_ELECTRONIC_SWITCH_INIT;

	for (int i = 0; i < m_electrinic_system.size(); ++i)
	{
		system_packet.data[i].idx = i;
		for (int idx = 0; idx < ON_OFF_SWITCH; ++idx)
			system_packet.data[i].value[idx] = m_electrinic_system[i].Get_On_Off_Switch_Correct_Value(idx);
	}

	for (int i = 0; i < JOIN_ROOM_MAX_USER; ++i)
		server.m_clients[in_player[i]].do_send(sizeof(system_packet), &system_packet);

	init_fix_object_and_life_chip();
}

void Room::End_Game(bool is_tagger_win)
{
	_room_state_lock.lock();
	_room_state = GAME_ROOM_STATE::READY;
	_room_state_lock.unlock();
	// 엔딩패킷처리 보내는곳

	sc_packet_game_end packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_GAME_END;
	packet.is_tagger_win = is_tagger_win;

}

void Room::Update_room_time()
{
	now_time = chrono::system_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 60 && m_tagger_id == -1 && _room_state == GAME_ROOM_STATE::PLAYING)
	{
		m_tagger_id = in_player[Select_Tagger()];
		cGameServer& server = *cGameServer::GetInstance();
		
		sc_packet_select_tagger packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_SELECT_TAGGER;
		packet.id = m_tagger_id;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;

		for (int i = 0; i < JOIN_ROOM_MAX_USER; ++i)
		{
			CLIENT& cl = *server.get_client_info(in_player[i]);
			cl.do_send(sizeof(packet), &packet);
		}
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 180 && false == m_first_skill_enable && _room_state == GAME_ROOM_STATE::PLAYING) // 술래 첫번째 스킬 활성화
	{
		cGameServer& server = *cGameServer::GetInstance();
		CLIENT& cl = *server.get_client_info(m_tagger_id);
		m_first_skill_enable = false;
		m_second_skill_enable = false;
		m_third_skill_enable = false;
		cl.set_first_skill_enable();
		sc_packet_select_tagger packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_SELECT_TAGGER;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 360 && false == m_second_skill_enable && _room_state == GAME_ROOM_STATE::PLAYING) // 술래 두번째 스킬 활성화
	{
		cGameServer& server = *cGameServer::GetInstance();
		CLIENT& cl = *server.get_client_info(m_tagger_id);
		m_second_skill_enable = true;
		cl.set_second_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 540 && false == m_third_skill_enable && _room_state == GAME_ROOM_STATE::PLAYING) // 술래 세번째 스킬 활성화
	{
		cGameServer& server = *cGameServer::GetInstance();
		CLIENT& cl = *server.get_client_info(m_tagger_id);
		m_third_skill_enable = true;
		cl.set_third_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}

	for (int i = 0; i < m_door_object.size(); ++i)
	{
		m_door_object[i].m_state_lock->lock();
		if (!m_door_object[i].m_door_open_start)
		{
			m_door_object[i].m_state_lock->unlock();
			continue;
		}
		
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now_time - m_door_object[i].m_door_open_start_time).count() >= m_door_object[i].m_door_open_duration && m_door_object[i].m_door_open_start == true)
		{
			m_door_object[i].m_door_open_start = false;
			m_door_object[i].set_boundingbox_check(false);
			m_door_object[i].m_state_lock->unlock();
		}
		else
			m_door_object[i].m_state_lock->unlock();
	}


	// 게임 종료를 확인하는 부분
	if (m_tagger_collect_chip == GAME_END_COLLECT_CHIP) // 술래가 정해진 갯수의 생명칩을 수거한 경우
	{
		End_Game(true);
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 720 && _room_state == GAME_ROOM_STATE::PLAYING) // 게임종료 확인(타임아웃)
	{
		End_Game(true);
	}
	duration_time = std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count();
}

bool Room::All_ElectronicSystem_Fixed()
{
	for (int i = 0; i < m_electrinic_system.size(); ++i)
	{
		if (!m_electrinic_system[i].Get_On_Off_Switch_Vaild())
			return false;
	}
	return true;
}