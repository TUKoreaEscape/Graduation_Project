#include "GameServer.h"


void cGameServer::Update_Session(int thread_number)
{
	cout << "thread number : " << thread_number << endl;
	while (true)
	{
		if (m_session_timer.Frame_Limit(30.f)) // 초당 1번 업데이트!
		{
			//cout << "Update Session!" << endl;
			for (int i = thread_number; i < MAX_ROOM; i+=2)
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
#if DEBUG
					for (int k = 0; k < 6; ++k)
					{
						if (rl.Get_Join_Member(k) != -1)
						{
							sc_other_player_move packet;
							packet.size = sizeof(packet);
							packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE;
							for (int in_id = 0; in_id < 6; ++in_id)
							{
								if (rl.Get_Join_Member(in_id) != -1 && rl.Get_Join_Member(in_id) != rl.Get_Join_Member(k))
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
								//else if (rl.Get_Join_Member(in_id) == rl.Get_Join_Member(k))
								//	send_calculate_move_packet(rl.Get_Join_Member(k));
								else
									packet.data[in_id].id = -1;
							}
							if (rl.Get_Number_of_users() != -1);
								m_clients[rl.Get_Join_Member(k)].do_send(sizeof(packet), &packet);
						}

					}
#endif
					break;
				}/**/

				case GAME_ROOM_STATE::PLAYING:
				{
					rl._room_state_lock.unlock();
					rl.Update_room_time();
					Update_OtherPlayer(i);
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


void cGameServer::Update_OtherPlayer(int room_number)
{
	Room& rl = *m_room_manager->Get_Room_Info(room_number);

	for (int k = 0; k < 6; ++k)
	{
		if (rl.Get_Join_Member(k) != -1)
		{
			sc_other_player_move packet;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE;
			for (int in_id = 0; in_id < 6; ++in_id)
			{
				if (rl.Get_Join_Member(in_id) != -1 && rl.Get_Join_Member(in_id) != rl.Get_Join_Member(k))
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
			if (rl.Get_Number_of_users() != -1);
			m_clients[rl.Get_Join_Member(k)].do_send(sizeof(packet), &packet);
		}

	}
}