#include "GameServer.h"


void cGameServer::Update_Session()
{
	while (true)
	{
		if (m_session_timer.Frame_Limit(1.f)) // 초당 1번 업데이트!
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
					rl.is_collision_player_to_object(rl.Get_Join_Member(0));
#if DEBUG
					/*for (int i = 0; i < 6; ++i)
					{
						if (rl.Get_Join_Member(i) != -1)
						{
							sc_update_user_packet packet;
							packet.size = 127;
							packet.type = SC_PACKET::SC_USER_UPDATE;
							packet.sub_size_mul = sizeof(packet) / 127;
							packet.sub_size_add = sizeof(packet) % 127;

							for (int id = 0; id < 6; ++i)
							{
								int t_id = rl.Get_Join_Member(id);
								packet.data[id].id = t_id;
								packet.data[id].input_key = m_clients[t_id].get_input_key();
								packet.data[id].look[0] = m_clients[t_id].get_look_x();
								packet.data[id].look[1] = m_clients[t_id].get_look_y();
								packet.data[id].look[2] = m_clients[t_id].get_look_z();
								packet.data[id].right[0] = m_clients[t_id].get_right_x();
								packet.data[id].right[1] = m_clients[t_id].get_right_y();
								packet.data[id].right[2] = m_clients[t_id].get_right_z();
								packet.data[id].position = m_clients[t_id].get_user_position();
							}
							m_clients[rl.Get_Join_Member(i)].do_send(sizeof(packet), &packet);
						}
					}*/
#endif
					break;
				}/**/

				case GAME_ROOM_STATE::PLAYING:
				{
					rl._room_state_lock.unlock();
					rl.Update_room_time();
					rl.Update_Player_Position();
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
