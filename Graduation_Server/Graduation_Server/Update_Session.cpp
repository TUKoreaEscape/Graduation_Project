#include "GameServer.h"


void cGameServer::Update_Session(int thread_number)
{
	int index = 0;
	while (true)
	{
		if (m_session_timer.Frame_Limit(SET_SERVER_UPDATE_FRAME)) // 초당 1번 업데이트!
		{
			//cout << "Update Session!" << endl;
			for (int i = thread_number; i < MAX_ROOM; i++)
			{
				Room& room = *m_room_manager->Get_Room_Info(i);
				room._room_state_lock.lock();
				switch (room._room_state)
				{
				case GAME_ROOM_STATE::FREE:
				{
					room._room_state_lock.unlock();
					break;
				}
				case GAME_ROOM_STATE::READY:
				{
					room._room_state_lock.unlock();
#if DEBUG
					Update_OtherPlayer(i, (float)1/SET_SERVER_UPDATE_FRAME);
#endif				
					break;
				}

				case GAME_ROOM_STATE::PLAYING:
				{
					room._room_state_lock.unlock();
					room.Update_room_time();
					Update_OtherPlayer(i, (float)1/SET_SERVER_UPDATE_FRAME);
					break;
				}

				case GAME_ROOM_STATE::END:
				{
					room._room_state_lock.unlock();
					break;
				}

				}

			}
		}
	}
}


void cGameServer::Update_OtherPlayer(int room_number, float elaspeTime)
{
	Room& room = *m_room_manager->Get_Room_Info(room_number);

	for (int i = 0; i < 6; ++i)
	{
		if (room.Get_Join_Member(i) != -1)
		{
			if (m_clients[room.Get_Join_Member(i)].get_user_attack_animation())
				m_clients[room.Get_Join_Member(i)].play_attack_animation(elaspeTime);

			if (m_clients[room.Get_Join_Member(i)].get_user_victim_animation())
				m_clients[room.Get_Join_Member(i)].play_victim_animation(elaspeTime);

			if (!m_clients[room.Get_Join_Member(i)].IsAttackAnimation())
				m_clients[room.Get_Join_Member(i)].set_attack_animation(false);

			if (!m_clients[room.Get_Join_Member(i)].IsVictimAnimation())
				m_clients[room.Get_Join_Member(i)].set_victim_animation(false);
			Update_Viewlist(room.Get_Join_Member(i), room_number);
		}
	}

	int index = 0;
	for (int k = 0; k < 6; ++k)
	{
		if (room.Get_Join_Member(k) != -1)
		{
			sc_other_player_move packet;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE;
			index = 0;
			for (auto& temp : packet.data)
				temp.id = -1;
			for (int in_id = 0; in_id < 6; ++in_id)
			{
				if (room.Get_Join_Member(in_id) != -1 && room.Get_Join_Member(in_id) != room.Get_Join_Member(k))
				{
					int this_id = room.Get_Join_Member(in_id);
					m_clients[this_id]._pos_lock.lock();
					packet.data[index].id = this_id;
					packet.data[index].input_key = m_clients[this_id].get_input_key();
					packet.data[index].look.x = m_clients[this_id].get_look_x();
					packet.data[index].look.y = m_clients[this_id].get_look_y();
					packet.data[index].look.z = m_clients[this_id].get_look_z();
					packet.data[index].right.x = m_clients[this_id].get_right_x();
					packet.data[index].right.y = m_clients[this_id].get_right_y();
					packet.data[index].right.z = m_clients[this_id].get_right_z();
					packet.data[index].position.x = static_cast<int>(m_clients[this_id].get_user_position().x * 10000);
					packet.data[index].position.y = static_cast<int>(m_clients[this_id].get_user_position().y * 10000);
					packet.data[index].position.z = static_cast<int>(m_clients[this_id].get_user_position().z * 10000);
					packet.data[index].is_jump = m_clients[this_id].get_user_is_jump();
					packet.data[index].is_attack = m_clients[this_id].get_user_attack_animation();
					packet.data[index].is_victim = m_clients[this_id].get_user_victim_animation();
					packet.data[index].is_collision_up_face = m_clients[this_id].get_user_collied_up_face();
					packet.data[index].active = true;
					m_clients[this_id]._pos_lock.unlock();
					index++;
				}

				else if (room.Get_Join_Member(in_id) == -1 && room.Get_Join_Member(in_id) != room.Get_Join_Member(k))
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
			m_clients[room.Get_Join_Member(k)].do_send(sizeof(packet), &packet);
		}

	}
}

void cGameServer::Update_Viewlist(const int id, const int room_number)
{
	for (auto player : m_room_manager->Get_Room_Info(room_number)->in_player)
	{
		if (player != -1 && player != id)
		{

			if (Is_near(id, player))
			{
				m_clients[id]._view_list_lock.lock();
				m_clients[id].view_list.insert(player);
				m_clients[id]._view_list_lock.unlock();
			}
			else
			{
				m_clients[id]._view_list_lock.lock();
				m_clients[id].view_list.erase(player);
				m_clients[id]._view_list_lock.unlock();
			}
		}
	}
}

bool cGameServer::Is_near(int a, int b)
{
	if (RANGE < abs(m_clients[a].get_user_position().x - m_clients[b].get_user_position().x)) return false;
	if (RANGE < abs(m_clients[a].get_user_position().z - m_clients[b].get_user_position().z)) return false;
	return true;
}