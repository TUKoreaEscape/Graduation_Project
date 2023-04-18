#include "GameServer.h"


void cGameServer::Update_Session(int thread_number)
{
	int index = 0;
	while (true)
	{
		if (m_session_timer.Frame_Limit(45.f)) // 초당 1번 업데이트!
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
					Update_OtherPlayer(i);
#endif				
					break;
				}

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
	int index = 0;
	for (int k = 0; k < 6; ++k)
	{
		if (rl.Get_Join_Member(k) != -1)
		{
			sc_other_player_move packet;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE;
			index = 0;
			for (auto& temp : packet.data)
				temp.id = -1;
			for (int in_id = 0; in_id < 6; ++in_id)
			{
				if (rl.Get_Join_Member(in_id) != -1 && rl.Get_Join_Member(in_id) != rl.Get_Join_Member(k))
				{
					int this_id = rl.Get_Join_Member(in_id);
					packet.data[index].id = this_id;
					packet.data[index].input_key = m_clients[this_id].get_input_key();
					packet.data[index].look.x = m_clients[this_id].get_look_x();
					packet.data[index].look.y = m_clients[this_id].get_look_y();
					packet.data[index].look.z = m_clients[this_id].get_look_z();
					packet.data[index].right.x = m_clients[this_id].get_right_x();
					packet.data[index].right.y = m_clients[this_id].get_right_y();
					packet.data[index].right.z = m_clients[this_id].get_right_z();
					packet.data[index].position.x = static_cast<short>(m_clients[this_id].get_user_position().x) * 100;
					packet.data[index].position.y = static_cast<short>(m_clients[this_id].get_user_position().y) * 100;
					packet.data[index].position.z = static_cast<short>(m_clients[this_id].get_user_position().z) * 100;
					packet.data[index].active = true;
					index++;
				}

				else if (rl.Get_Join_Member(in_id) == -1 && rl.Get_Join_Member(in_id) != rl.Get_Join_Member(k))
				{
					packet.data[index].id = -1;
					packet.data[index].look.x = 0;
					packet.data[index].look.y = 0;
					packet.data[index].look.z = 0;
					packet.data[index].right.x = 0;
					packet.data[index].right.y = 0;
					packet.data[index].right.z = 0;
					packet.data[index].position.x = -100 * 100;
					packet.data[index].position.y = 1000 * 10;
					packet.data[index].position.z = -100 * 100;
					packet.data[index].active = false;
					index++;
				}
			}
			//if (rl.Get_Number_of_users() < 5);
			m_clients[rl.Get_Join_Member(k)].do_send(sizeof(packet), &packet);
		}

	}
}