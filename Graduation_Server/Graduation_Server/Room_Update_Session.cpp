#include "Room.h"
#include "GameServer.h"

int Room::Select_Tagger()
{
	mt19937 engine((unsigned int)time(NULL));

	return engine() % 6;
}

void Room::Start_Game()
{
	start_time = chrono::system_clock::now();
	_room_state = GAME_ROOM_STATE::PLAYING;

	cGameServer& server = cGameServer::GetInstance();

	for (auto p : in_player)
	{
		CLIENT& player = *server.get_client_info(p);
		player.set_user_position({ 100,100,100 });
	}
}

void Room::End_Game()
{
	_room_state_lock.lock();
	_room_state = GAME_ROOM_STATE::END;
	_room_state_lock.unlock();

	cout << "게임이 종료되었습니다." << endl;
	// 엔딩패킷처리 보내는곳

}

void Room::Update_room_time()
{
	now_time = chrono::system_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 60 && m_tagger_id == -1)
	{
		m_tagger_id = in_player[Select_Tagger()];
		cGameServer& server = cGameServer::GetInstance();
		CLIENT& cl = *server.get_client_info(m_tagger_id);

		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}

	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(m_tagger_id);


	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 180 && false == m_first_skill_enable) // 술래 첫번째 스킬 활성화
	{
		m_first_skill_enable = true;
		cl.set_first_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 360 && false == m_second_skill_enable) // 술래 두번째 스킬 활성화
	{
		m_second_skill_enable = true;
		cl.set_second_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 540 && false == m_third_skill_enable) // 술래 세번째 스킬 활성화
	{
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


	// 게임 종료를 확인하는 부분
	if (m_tagger_collect_chip == GAME_END_COLLECT_CHIP) // 술래가 정해진 갯수의 생명칩을 수거한 경우
	{
		End_Game();
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 720) // 게임종료 확인(타임아웃)
	{
		End_Game();
	}
	duration_time = std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count();
}