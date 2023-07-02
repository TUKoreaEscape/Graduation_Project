#include "GameServer.h"

void cGameServer::Timer()
{
	TIMER_EVENT ev{};

	std::priority_queue<TIMER_EVENT> timer_queue;

	while (!server_end)
	{
		//cout << "�̺�Ʈ������ ������" << endl;
		auto current_time = chrono::system_clock::now();

		if (!timer_queue.empty()) {
			if (timer_queue.top().event_time <= current_time) {
				ev = timer_queue.top();
				timer_queue.pop();
				Process_Event(ev);
			}
		}

		if (m_timer_queue.try_pop(ev)) {
			if (ev.event_time > current_time) {
				if (ev.event_type == EventType::CHECK_NUM_OF_SERVER_ACCEPT_USER)
				{
					timer_queue.push(ev);
					//std::this_thread::sleep_for(4ms);
					std::this_thread::sleep_for(4ms);
					continue;
				}
				else {
					//m_room_manager->Get_Room_Info(ev.room_number)->_room_state_lock.lock();
					if (m_room_manager->Get_Room_Info(ev.room_number)->_room_state == GAME_ROOM_STATE::PLAYING || m_room_manager->Get_Room_Info(ev.room_number)->_room_state == GAME_ROOM_STATE::READY)
					{
						timer_queue.push(ev);
						std::this_thread::sleep_for(4ms);
						continue;
					}
				}
			}
			else {
				Process_Event(ev);
				continue;
			}
		}
		else
			std::this_thread::sleep_for(4ms);
	}
}


void cGameServer::Process_Event(const TIMER_EVENT& ev)
{

	switch (ev.event_type)
	{
	case EventType::CHECK_NUM_OF_SERVER_ACCEPT_USER:
	{
		int number_of_users_in_server = 0;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (m_clients[i].get_state() == CLIENT_STATE::ST_FREE)
				continue;
			number_of_users_in_server++;
		}
		join_member = number_of_users_in_server;
		cout << "Number of users accessing the server : " << number_of_users_in_server << "                                                            " << "\r";

		TIMER_EVENT next_ev;
		next_ev.event_type = EventType::CHECK_NUM_OF_SERVER_ACCEPT_USER;
		next_ev.event_time = chrono::system_clock::now() + 5s;

		m_timer_queue.push(next_ev);
		return;
	}
	
	case EventType::UPDATE_MOVE:
	{
		//cout << "event update ����" << endl;
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_UPDATE_PLAYER_MOVE;
		memcpy(&over->m_wsa_buf, &ev, sizeof(ev));
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	case EventType::SELECT_TAGGER:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_SELECT_TAGGER;
		memcpy(&over->m_wsa_buf, &ev, sizeof(ev));
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	case EventType::PLAYER_ATTACK:
	{
		m_clients[ev.obj_id].set_attack_animation(false);
		return;
	}

	case EventType::PLAYER_VICTIM:
	{
		m_clients[ev.obj_id].set_victim_animation(false);
		return;
	}

	case EventType::OPEN_TAGGER_SKILL_FIRST:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_FIRST_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	case EventType::OPEN_TAGGER_SKILL_SECOND:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_SECOND_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	case EventType::OPEN_TAGGER_SKILL_THIRD:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_THIRD_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	case EventType::OPEN_DOOR:
	{
		Room& rl = *m_room_manager->Get_Room_Info(ev.room_number);

		rl.m_door_object[ev.obj_id].m_check_bounding_box = false;
		rl.m_door_object[ev.obj_id].m_door_open_start = false;
		return;
	}

	case EventType::CLOSE_DOOR:
	{
		Room& rl = *m_room_manager->Get_Room_Info(ev.room_number);

		rl.m_door_object[ev.obj_id].m_check_bounding_box = true;
		rl.m_door_object[ev.obj_id].m_door_close_start = false;
		return;
	}

	case EventType::OPEN_ELECTRONIC:
	{
		return;
	}

	case EventType::CLOSE_ELECTRONIC:
	{

		return;
	}

	case EventType::GAME_END:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_GAME_END;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		return;
	}

	}

}