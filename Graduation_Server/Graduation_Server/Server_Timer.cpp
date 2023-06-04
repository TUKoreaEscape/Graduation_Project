#include "GameServer.h"

void cGameServer::Timer()
{
	TIMER_EVENT ev{};

	std::priority_queue<TIMER_EVENT> timer_queue;

	while (true)
	{
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
	case EventType::DOOR_TIME:
	{
		EXP_OVER* over = new EXP_OVER;
		over->m_comp_op = OP_DOOR_OPEN;

		break;
	}

	}

}