#include "GameServer.h"

void cGameServer::Timer()
{
	TIMER_EVENT ev{};

	std::priority_queue<TIMER_EVENT> timer_queue;

	while (true)
	{
		//cout << "이벤트쓰레드 도는중" << endl;
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
				timer_queue.push(ev);
				std::this_thread::sleep_for(1ms);
				continue;
			}
			else {
				Process_Event(ev);
				continue;
			}
		}
		std::this_thread::sleep_for(1ms);
	}
}

void cGameServer::Process_Event(const TIMER_EVENT& ev)
{

	switch (ev.event_type)
	{
	
	case EventType::UPDATE_MOVE:
	{
		cout << "event update 들어옴" << endl;
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_UPDATE_PLAYER_MOVE;
		memcpy(&over->m_wsa_buf, &ev, sizeof(ev));
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		break;
	}

	case EventType::SELECT_TAGGER:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_SELECT_TAGGER;
		memcpy(&over->m_wsa_buf, &ev, sizeof(ev));
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		break;
	}

	case EventType::OPEN_TAGGER_SKILL_FIRST:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_FIRST_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		break;
	}

	case EventType::OPEN_TAGGER_SKILL_SECOND:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_SECOND_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		break;
	}

	case EventType::OPEN_TAGGER_SKILL_THIRD:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_TYPE::OP_THIRD_TAGGER_SKILL_OPEN;
		PostQueuedCompletionStatus(C_IOCP::m_h_iocp, 1, ev.room_number, &over->m_wsa_over);
		break;
	}

	case EventType::OPEN_DOOR:
	{
		Room& rl = *m_room_manager->Get_Room_Info(ev.room_number);

		rl.m_door_object[ev.obj_id].m_check_bounding_box = false;
		rl.m_door_object[ev.obj_id].m_door_open_start = false;
		break;
	}

	case EventType::CLOSE_DOOR:
	{
		Room& rl = *m_room_manager->Get_Room_Info(ev.room_number);

		rl.m_door_object[ev.obj_id].m_check_bounding_box = true;
		rl.m_door_object[ev.obj_id].m_door_close_start = false;
		break;
	}

	}

}