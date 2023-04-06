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
					rl.is_collision_player_to_object(rl.Get_Join_Member(0));
#if DEBUG

					for (int i = 0; i < 6; ++i)
					{
						if (rl.Get_Join_Member(i) != -1)
						{
							sc_other_player_move packet;
							packet.size = sizeof(packet);
							packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE;
							for (int in_id = 0; i < 6; ++i)
							{
								if (rl.Get_Join_Member(in_id) != -1)
								{
									int this_id = rl.Get_Join_Member(in_id);
									packet.data[in_id].id = this_id;
									packet.data[in_id].input_key = m_clients[this_id].get_input_key();
									packet.data[in_id].look.x = m_clients[this_id].get_look_x();
									packet.data[in_id].look.y = m_clients[this_id].get_look_y();
									packet.data[in_id].look.z = m_clients[this_id].get_look_z();
									packet.data[in_id].right.x = m_clients[this_id].get_right_x();
									packet.data[in_id].right.y = m_clients[this_id].get_right_y();
									packet.data[in_id].right.z = m_clients[this_id].get_right_z();
									packet.data[in_id].position.x = m_clients[this_id].get_user_position().x * 100;
									packet.data[in_id].position.y = m_clients[this_id].get_user_position().y * 100;
									packet.data[in_id].position.z = m_clients[this_id].get_user_position().z * 100;
									packet.data[in_id].active = true;
								}
								else
									packet.data[in_id].id = -1;
							}
							m_clients[rl.Get_Join_Member(i)].do_send(sizeof(packet), &packet);
						}

					}
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
