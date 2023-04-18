#include "GameServer.h"

void cGameServer::Process_Create_ID(int c_id, void* buff) // ��û���� ID������Ŷ ó��
{
	int reason = 0;

	cs_packet_create_id* packet = reinterpret_cast<cs_packet_create_id*>(buff);

	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->create_id(stringToWstring(stringID), stringToWstring(stringPW));

	if (reason == 1) // id ���� ����
	{
		//m_database->Save_Customizing(stringToWstring(stringID), init_data);
		send_create_id_ok_packet(c_id);
	}
	else // id ���� ����
		send_create_id_fail_packet(c_id, reason);
}

void cGameServer::Process_Move(const int user_id, void* buff) // ��û���� ĳ���� �̵��� ó��
{
	cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buff);
	m_clients[user_id].set_user_velocity(packet->velocity);
	m_clients[user_id].set_user_yaw(packet->yaw);
	m_clients[user_id].update_rotation(packet->yaw);
	m_clients[user_id].set_look(packet->look);
	m_clients[user_id].set_right(packet->right);
	m_clients[user_id].set_inputKey(packet->input_key);
	Room& join_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	XMFLOAT3 current_player_position = m_clients[user_id].get_user_position();

	XMFLOAT3 current_shift = packet->xmf3Shift;
	XMFLOAT3 calculate_player_position = Add(current_player_position, current_shift);

	m_clients[user_id].set_user_position(calculate_player_position);
	m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	if (m_clients[user_id].get_user_position().y < 0)
	{
		calculate_player_position.y = 0;
		m_clients[user_id].set_user_position(calculate_player_position);
		m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	}
	//cout << "������ ĳ���� ��ǥ : " << m_clients[user_id].get_user_position().x << ", " << m_clients[user_id].get_user_position().y << ", " << m_clients[user_id].get_user_position().z << endl;
	
	if (m_clients[user_id].get_join_room_number() >= 0) {
		CollisionInfo player_check = join_room.is_collision_player_to_player(user_id, current_player_position, current_shift);
		if (player_check.is_collision)
		{
			//cout << "�����̵����� : " << player_check.SlidingVector.x << ", " << player_check.SlidingVector.y << ", " << player_check.SlidingVector.z << endl;
			calculate_player_position = current_player_position;
			calculate_player_position = Add(current_player_position, player_check.SlidingVector);
			m_clients[user_id].set_user_position(calculate_player_position);
			m_clients[user_id].update_bounding_box_pos(calculate_player_position);
			if (m_clients[user_id].get_user_position().y < 0)
			{
				calculate_player_position.y = 0;
				m_clients[user_id].set_user_position(calculate_player_position);
				m_clients[user_id].update_bounding_box_pos(calculate_player_position);
			}
			current_shift = player_check.SlidingVector;
			//cout << "������ ĳ���� ��ǥ : " << m_clients[user_id].get_user_position().x << ", " << m_clients[user_id].get_user_position().y << ", " << m_clients[user_id].get_user_position().z << endl;
		}

		CollisionInfo wall_check = join_room.is_collision_wall_to_player(user_id, current_player_position, current_shift);
		if (wall_check.is_collision)
		{
			calculate_player_position = current_player_position;
			calculate_player_position = Add(current_player_position, wall_check.SlidingVector);
			m_clients[user_id].set_user_position(calculate_player_position);
			m_clients[user_id].update_bounding_box_pos(calculate_player_position);
			if (m_clients[user_id].get_user_position().y < 0)
			{
				calculate_player_position.y = 0;
				m_clients[user_id].set_user_position(calculate_player_position);
				m_clients[user_id].update_bounding_box_pos(calculate_player_position);
			}
			current_shift = wall_check.SlidingVector;
		}

		CollisionInfo object_check = join_room.is_collision_player_to_object(user_id, current_player_position, current_shift);
		if (object_check.is_collision)
		{
			// ������ ������Ʈ�� �浹�Ѱ��� ó���ϴ� �κ��Դϴ�.
		}
	}

#if !DEBUG
	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
		send_move_packet(*ptr, user_id, packet->position);
#endif

#if DEBUG
	send_calculate_move_packet(user_id); // -> �̵��� ���Ѱ� ó���Ͽ� Ŭ�󿡰� ������
#endif
}

void cGameServer::Process_Chat(const int user_id, void* buff)
{
	cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buff);
	char mess[MAX_CHAT_SIZE];

	strcpy_s(mess, packet->message);

	// ���� �濡 �ִ� �������׸� �޼��� ���� ����
	//m_clients[user_id]._room_list_lock.lock();
	//for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
	//	send_chat_packet(*ptr, user_id, mess);
	//m_clients[user_id]._room_list_lock.unlock();

	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());


	for (int i = 0; i < 6; ++i)
	{
		if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
			send_chat_packet(rl.Get_Join_Member(i), user_id, mess);
	}
}

void cGameServer::Process_Create_Room(const unsigned int _user_id) // ��û���� ���ο� �� ����
{
	m_clients[_user_id].set_join_room_number(m_room_manager->Create_room(_user_id));
	m_clients[_user_id]._state_lock.lock();
	m_clients[_user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
	m_clients[_user_id]._state_lock.unlock();
	m_clients[_user_id].set_bounding_box(m_clients[_user_id].get_user_position(), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT4(0, 0, 0, 1));
	send_create_room_ok_packet(_user_id, m_clients[_user_id].get_join_room_number());
	send_put_player_data(_user_id);
}

void cGameServer::Process_Join_Room(const int user_id, void* buff)
{
	cs_packet_join_room* packet = reinterpret_cast<cs_packet_join_room*>(buff);
	cout << "Process_Join_Room" << endl;

	if (m_room_manager->Get_Room_Info(packet->room_number)->_room_state == GAME_ROOM_STATE::READY)
	{
		if (m_room_manager->Join_room(user_id, packet->room_number))
		{
			m_clients[user_id].set_join_room_number(packet->room_number);
			m_clients[user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
			send_put_player_data(user_id);
			Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
				{
					m_clients[user_id]._room_list_lock.lock();
					m_clients[user_id].room_list.insert(rl.Get_Join_Member(i));
					m_clients[user_id]._room_list_lock.unlock();
					send_put_other_player(rl.Get_Join_Member(i), user_id);
				}
			}


			// ���� ���⿡ �� �濡 �����ϴ� ��� ������� ���� �����ߴ��� ������ �����ؾ���!
			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
				{
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.lock();
					m_clients[rl.Get_Join_Member(i)].room_list.insert(user_id);
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.unlock();
					send_put_other_player(user_id, rl.Get_Join_Member(i));
				}
			}
			m_clients[user_id].set_user_position(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_clients[user_id].set_bounding_box(m_clients[user_id].get_user_position(), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT4(0, 0, 0, 1));
			send_join_room_success_packet(user_id);
			cout << "send_join_room_success_packet" << endl;
		}
		else {
			send_join_room_fail_packet(user_id);
			cout << "send_join_room_fail_packet" << endl;
		}
	}
	else
		send_join_room_fail_packet(user_id);

}

void cGameServer::Process_Exit_Room(const int user_id, void* buff)
{
	cs_packet_request_exit_room* packet = reinterpret_cast<cs_packet_request_exit_room*>(buff);

	if (m_clients[user_id].get_join_room_number() != -1) {
		Room& cl_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
		cl_room.Exit_Player(user_id);
		m_clients[user_id].set_join_room_number(-1);
		m_clients[user_id].set_state(CLIENT_STATE::ST_LOBBY);

		for (auto& p : m_clients[user_id].room_list)
		{
			m_clients[p]._room_list_lock.lock();
			m_clients[p].room_list.erase(m_clients[p].room_list.find(user_id));
			m_clients[p]._room_list_lock.unlock();
		}

		m_clients[user_id]._room_list_lock.lock();
		m_clients[user_id].room_list.clear();
		m_clients[user_id]._room_list_lock.unlock();

		// ���� ������ �κ��̹Ƿ� �� �������� �ٽ� ���������!
		sc_packet_request_room_info send_packet;
		int room_number;
		for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1�������� 10���� ���� ���δٰ� ����, �������� ������ �غ���
		{
			room_number = i + (packet->request_page * MAX_ROOM_INFO_SEND);
			Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
			send_packet.room_info[i].room_number = room_number;
			send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
			send_packet.room_info[i].state = get_room_info._room_state;
			get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 20);
		}

		if (sizeof(send_packet) >= CHECK_MAX_PACKET_SIZE)
		{
			send_packet.size = CHECK_MAX_PACKET_SIZE;
			send_packet.sub_size_mul = sizeof(sc_packet_request_room_info) / CHECK_MAX_PACKET_SIZE;
			send_packet.sub_size_add = sizeof(sc_packet_request_room_info) % CHECK_MAX_PACKET_SIZE;
		}
		else
			send_packet.size = sizeof(sc_packet_request_room_info);
		send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
		m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
	}
}

void cGameServer::Process_Ready(const int user_id, void* buff)
{
	cs_packet_ready* packet = reinterpret_cast<cs_packet_ready*>(buff);

	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	cout << "id : " << user_id << "�� ready��" << endl;
	cout << rl.All_Player_Ready() << endl;
	rl.SetReady(packet->ready_type, user_id);
	if (rl.All_Player_Ready())
	{
		for (auto p : rl.in_player) {
			send_game_start_packet(p);
			m_clients[p]._state_lock.lock();
			m_clients[p].set_state(CLIENT_STATE::ST_INGAME);
			m_clients[p]._state_lock.unlock();
		}
		// ��� �÷��̾ ���� �� ��� ���� ������ �����ϰ� �ٲ�����ϴ� �κ�!
		rl._room_state_lock.lock();
		rl._room_state_lock.unlock();
	}
}

void cGameServer::Process_Game_Start(const int user_id)
{
	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	rl.SetLoading(true, user_id);
	if (rl.All_Player_Loading())
		rl.Start_Game();
}

void cGameServer::Process_Request_Room_Info(const int user_id, void* buff)
{
	cs_packet_request_all_room_info* packet = reinterpret_cast<cs_packet_request_all_room_info*>(buff);

	sc_packet_request_room_info send_packet;

	int room_number;
	for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1�������� 10���� ���� ���δٰ� ����, �������� ������ �غ���
	{
		room_number = i + (packet->request_page * MAX_ROOM_INFO_SEND);
		Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
		send_packet.room_info[i].room_number = room_number;
		send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
		send_packet.room_info[i].state = get_room_info._room_state;
		get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 20);
	}

	if (sizeof(sc_packet_request_room_info) >= 127)
	{
		send_packet.size = 127;
		send_packet.sub_size_mul = sizeof(sc_packet_request_room_info) / 127;
		send_packet.sub_size_add = sizeof(sc_packet_request_room_info) % 127;
	}
	else
		send_packet.size = sizeof(sc_packet_request_room_info);
	send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
	m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
}

void cGameServer::Process_User_Login(int c_id, void* buff) // �α��� ��û
{
	int reason = 0;

	cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buff);
	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->check_login(stringToWstring(stringID), stringToWstring(stringPW));
	if (reason == 1) // reason 0 : id�� �������� ���� / reason 1 : ���� / reason 2 : pw�� Ʋ��
	{
		m_clients[c_id].set_name(packet->id);
		m_clients[c_id].set_login_state(Y_LOGIN);
		m_clients[c_id].set_state(CLIENT_STATE::ST_LOBBY);
		m_clients[c_id].m_customizing->Load_Customizing_Data_To_DB(stringToWstring(stringID));
		send_login_ok_packet(c_id);
	}
	else
	{
		if (reason == 0)
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::INVALID_ID);
		else
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::WRONG_PW);
	}
}

void cGameServer::Process_Voice_Data(int user_id)
{
	// ���⼭ ��ū�� �������ִ°� ���ƺ���.
	send_voice_data(user_id);
}

void cGameServer::Process_Customizing(const int user_id, void* buff)
{
	cs_packet_customizing_update* packet = reinterpret_cast<cs_packet_customizing_update*>(buff);

	m_clients[user_id].m_customizing->Set_Body_Custom(static_cast<BODIES>(packet->body));
	m_clients[user_id].m_customizing->Set_Body_Part_Custom(static_cast<BODYPARTS>(packet->body_parts));
	m_clients[user_id].m_customizing->Set_Eyes_Custom(static_cast<EYES>(packet->eyes));
	m_clients[user_id].m_customizing->Set_Gloves_Custom(static_cast<GLOVES>(packet->gloves));
	m_clients[user_id].m_customizing->Set_Head_Custom(static_cast<HEADS>(packet->head));
	m_clients[user_id].m_customizing->Set_Head_Part_Custom(static_cast<HEADPARTS>(packet->head_parts));
	m_clients[user_id].m_customizing->Set_Mouthandnoses_Custom(static_cast<MOUTHANDNOSES>(packet->mouthandnoses));
	m_clients[user_id].m_customizing->Set_Tail_Custom(static_cast<TAILS>(packet->tails));
}