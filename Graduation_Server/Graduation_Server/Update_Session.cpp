#include "GameServer.h"


void cGameServer::Update_Session()
{
	while (true)
	{
		if (m_session_timer.Frame_Limit(30.f)) // 초당 1번 업데이트!
		{
			//cout << "Update Session!" << endl;
			for (int i = 0; i < MAX_ROOM; ++i)
			{
				Room& rl = *m_room_manager->Get_Room_Info(i);
				rl._room_state_lock.lock();
				switch (rl._room_state)
				{
				case GAME_ROOM_STATE::FREE:
				{
					rl._room_state_lock.unlock();
					break;
				}

				case GAME_ROOM_STATE::READY:
				{
					rl._room_state_lock.unlock();
					break;
				}/**/

				case GAME_ROOM_STATE::PLAYING:
				{
					rl._room_state_lock.unlock();
					rl.Update_room_time();
					break;
				}

				case GAME_ROOM_STATE::END:
				{
					rl._room_state_lock.unlock();
					break;
				}

				}

			}
		}
	}
}
