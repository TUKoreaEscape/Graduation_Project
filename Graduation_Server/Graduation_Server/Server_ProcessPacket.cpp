#include "GameServer.h"

void cGameServer::Process_Create_ID(int c_id, void* buff) // 요청받은 ID생성패킷 처리
{
	int reason = 0;

	cs_packet_create_id* packet = reinterpret_cast<cs_packet_create_id*>(buff);

	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->create_id(stringToWstring(stringID), stringToWstring(stringPW));

	cout << "reason : " << reason << endl;
	if (reason == 1) // id 생성 성공
		send_create_id_ok_packet(c_id);
	else // id 생성 실패
		send_create_id_fail_packet(c_id, reason);
}

void cGameServer::Process_Move(const int user_id, void* buff) // 요청받은 캐릭터 이동을 처리
{
	cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buff);

	m_clients[user_id].set_user_velocity(packet->velocity);
	m_clients[user_id].set_user_yaw(packet->yaw);
	m_clients[user_id].update_rotation(packet->yaw);
	Room& join_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	XMFLOAT3 current_player_position = m_clients[user_id].get_user_position();
	XMFLOAT3 current_shift = packet->xmf3Shift;
	XMFLOAT3 calculate_player_position = Add(current_player_position, current_shift);

	m_clients[user_id].set_user_position(calculate_player_position);
	m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	//cout << "적용전 캐릭터 좌표 : " << m_clients[user_id].get_user_position().x << ", " << m_clients[user_id].get_user_position().y << ", " << m_clients[user_id].get_user_position().z << endl;
	CollisionInfo player_check = join_room.is_collision_player_to_player(user_id, current_player_position, packet->xmf3Shift);
	if (player_check.is_collision)
	{
		//cout << "슬라이딩벡터 : " << player_check.SlidingVector.x << ", " << player_check.SlidingVector.y << ", " << player_check.SlidingVector.z << endl;
		calculate_player_position = current_player_position;
		calculate_player_position = Add(current_player_position, player_check.SlidingVector);
		m_clients[user_id].set_user_position(calculate_player_position);
		m_clients[user_id].update_bounding_box_pos(calculate_player_position);
		//cout << "적용후 캐릭터 좌표 : " << m_clients[user_id].get_user_position().x << ", " << m_clients[user_id].get_user_position().y << ", " << m_clients[user_id].get_user_position().z << endl;
	}

	CollisionInfo wall_check = join_room.is_collision_wall_to_player(user_id, current_player_position, packet->xmf3Shift);
	if (wall_check.is_collision)
	{
		calculate_player_position = current_player_position;
		calculate_player_position = Add(current_player_position, wall_check.SlidingVector);
		m_clients[user_id].set_user_position(calculate_player_position);
		m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	}

	if (join_room.is_collision_player_to_object(user_id))
	{
		// 이쪽은 오브젝트와 충돌한것을 처리하는 부분입니다.
	}

	//for (auto ptr = m_clients[user_id].view_list.begin(); ptr != m_clients[user_id].view_list.end(); ++ptr)
	//	send_move_packet(*ptr, user_id, packet->position);
#if !DEBUG
	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
		send_move_packet(*ptr, user_id, packet->position);
#endif

#if DEBUG
	send_move_packet(user_id, user_id, *packet, current_player_position);
	//for (int i = 0; i < MAX_USER; ++i)
	//{
	//	if (m_clients[i].get_id() != -1 && m_clients[i].get_id() != user_id)
	//	{
	//		cout << "send_move" << endl;
	//		send_move_packet(i, user_id, calculate_player_position);
	//	}
	//}

	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
	{
		send_move_packet(*ptr, user_id, *packet, current_player_position);
	}

#endif
}

void cGameServer::Process_Rotate(const int user_id, void* buff)
{
	cs_packet_player_rotate* packet = reinterpret_cast<cs_packet_player_rotate*>(buff);

	m_clients[user_id].set_user_yaw(packet->yaw);
	float radian = XMConvertToRadians(packet->yaw);

	XMFLOAT4 calculate;
	XMStoreFloat4(&calculate, XMQuaternionRotationRollPitchYaw(0.0f, radian, 0.0f));
	m_clients[user_id].update_bounding_box_orientation(calculate);
	m_clients[user_id]._room_list_lock.lock();
	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
	{
		send_rotate_packet(*ptr, user_id, *packet);
	}
	m_clients[user_id]._room_list_lock.unlock();
}

void cGameServer::Process_Chat(const int user_id, void* buff)
{
	cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buff);
	char mess[MAX_CHAT_SIZE];

	strcpy_s(mess, packet->message);

	// 같은 방에 있는 유저한테만 메세지 보낼 예정
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

void cGameServer::Process_Create_Room(const unsigned int _user_id) // 요청받은 새로운 방 생성
{
	m_clients[_user_id].set_join_room_number(m_room_manager->Create_room(_user_id));
	m_clients[_user_id]._state_lock.lock();
	m_clients[_user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
	m_clients[_user_id]._state_lock.unlock();
	m_clients[_user_id].set_bounding_box(m_clients[_user_id].get_user_position(), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT4(0, 0, 0, 1));
	send_create_room_ok_packet(_user_id, m_clients[_user_id].get_join_room_number());
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
			Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
				{
					m_clients[user_id]._room_list_lock.lock();
					m_clients[user_id].room_list.insert(rl.Get_Join_Member(i));
					m_clients[user_id]._room_list_lock.unlock();
				}
			}


			// 이제 여기에 그 방에 존재하는 모든 사람에게 누가 접속했는지 정보를 전달해야함!
			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
				{
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.lock();
					m_clients[rl.Get_Join_Member(i)].room_list.insert(user_id);
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.unlock();
				}
			}
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

		// 방을 나가면 로비이므로 방 정보들을 다시 보내줘야함!
		sc_packet_request_room_info send_packet;
		int room_number;
		for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1페이지당 10개의 방이 보인다고 가정, 룸정보를 전송할 준비함
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
	cout << "id : " << user_id << "가 ready함" << endl;
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
		// 모든 플레이어가 레디가 된 경우 이제 게임을 시작하게 바꿔줘야하는 부분!
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
	for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1페이지당 10개의 방이 보인다고 가정, 룸정보를 전송할 준비함
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

void cGameServer::Process_User_Login(int c_id, void* buff) // 로그인 요청
{
	int reason = 0;

	cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buff);
	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->check_login(stringToWstring(stringID), stringToWstring(stringPW));
	if (reason == 1) // reason 0 : id가 존재하지 않음 / reason 1 : 성공 / reason 2 : pw가 틀림
	{
		send_login_ok_packet(c_id);
		m_clients[c_id].set_name(packet->id);
		m_clients[c_id].set_login_state(Y_LOGIN);
		m_clients[c_id].set_state(CLIENT_STATE::ST_LOBBY);
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
	// 여기서 토큰을 제작해주는게 좋아보임.
	send_voice_data(user_id);
}