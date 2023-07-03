#include "GameServer.h"

void cGameServer::Process_Create_ID(int c_id, void* buff) // 요청받은 ID생성패킷 처리
{
	int reason = 0;

	cs_packet_create_id* packet = reinterpret_cast<cs_packet_create_id*>(buff);

	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;

	DB_Request request;
	request.request_id = c_id;
	request.request_name = stringToWstring(stringID);
	request.request_pw = stringToWstring(stringPW);
	request.type = REQUEST_CREATE_ID;

	m_database->insert_request(request);
}

void cGameServer::Process_Move(const int user_id, void* buff) // 요청받은 캐릭터 이동을 처리
{
	auto start_time = chrono::steady_clock::now();
	cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buff);
	m_clients[user_id].set_user_velocity(packet->velocity);
	m_clients[user_id].set_user_yaw(packet->yaw);
	m_clients[user_id].update_rotation(packet->yaw);
	m_clients[user_id].set_look(packet->look);
	m_clients[user_id].set_right(packet->right);
	m_clients[user_id].set_inputKey(packet->input_key);
	m_clients[user_id].set_isjump(packet->is_jump);
	Room& join_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	XMFLOAT3 current_player_position = m_clients[user_id].get_user_position();

	XMFLOAT3 current_shift = packet->xmf3Shift;
	XMFLOAT3 calculate_player_position = Add(current_player_position, current_shift);
	bool collision_up_face = false;

	if (m_clients[user_id].get_user_position().y < 0)
		calculate_player_position.y = 0;

	if (m_clients[user_id].get_join_room_number() >= 0) {
		CollisionInfo player_check = join_room.is_collision_player_to_player(user_id, current_player_position, current_shift);
		if (player_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, player_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = player_check.SlidingVector;
			if (player_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo wall_check = join_room.is_collision_wall_to_player(user_id, current_player_position, current_shift);
		if (wall_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, wall_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = wall_check.SlidingVector;
			if (wall_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo fix_object_check = join_room.is_collision_fix_object_to_player(user_id, current_player_position, current_shift);
		if (fix_object_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, fix_object_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = fix_object_check.SlidingVector;
			if (fix_object_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo door_check = join_room.is_collision_player_to_door(user_id, current_player_position, current_shift);
		if (door_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, door_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = door_check.SlidingVector;
			if (door_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo object_check = join_room.is_collision_player_to_object(user_id, current_player_position, current_shift);
		if (object_check.is_collision)
		{
			// 이쪽은 오브젝트와 충돌한것을 처리하는 부분입니다.
			if (object_check.collision_face_num == 4)
				collision_up_face = true;
		}
	}

	m_clients[user_id].set_user_position(calculate_player_position);
	m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	m_clients[user_id].set_collied_up_face(collision_up_face);

	send_calculate_move_packet(user_id); // -> 이동에 대한걸 처리하여 클라에게 보내줌
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	if (m_clients[user_id].m_befor_send_move == false)
	{
		m_clients[user_id].m_befor_send_move = true;
		for (auto p : room.in_player)
		{
			if (p == user_id)
				continue;
			if (p == -1)
				continue;
			send_move_packet(p, user_id);
		}
	}
	else
		m_clients[user_id].m_befor_send_move = false;
	auto end_time = chrono::steady_clock::now();

	//cout << chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << "ms" << endl;
}

void cGameServer::Process_Chat(const int user_id, void* buff)
{
	cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buff);
	char mess[MAX_CHAT_SIZE];

	strcpy_s(mess, packet->message);

	// 같은 방에 있는 유저한테만 메세지 보낼 예정

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());


	for (int i = 0; i < 6; ++i)
	{
		if (room.Get_Join_Member(i) != -1 && room.Get_Join_Member(i) != user_id)
			send_chat_packet(room.Get_Join_Member(i), user_id, mess);
	}
}

void cGameServer::Process_Create_Room(const unsigned int _user_id, void* buff) // 요청받은 새로운 방 생성
{
	cs_packet_create_room* packet = reinterpret_cast<cs_packet_create_room*>(buff);

	m_clients[_user_id].set_join_room_number(m_room_manager->Create_room(_user_id, packet->room_number));
	m_clients[_user_id]._state_lock.lock();
	m_clients[_user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
	m_clients[_user_id]._state_lock.unlock();
	send_create_room_ok_packet(_user_id, m_clients[_user_id].get_join_room_number());
	//send_put_player_data(_user_id);
	m_clients[_user_id].set_bounding_box(m_clients[_user_id].get_user_position(), XMFLOAT3(0.7f, 1.f, 0.7f), XMFLOAT4(0, 0, 0, 1));


	m_clients[_user_id].set_item_own(GAME_ITEM::ITEM_DRILL, false);
	m_clients[_user_id].set_item_own(GAME_ITEM::ITEM_HAMMER, false);
	m_clients[_user_id].set_item_own(GAME_ITEM::ITEM_PLIERS, false);
	m_clients[_user_id].set_item_own(GAME_ITEM::ITEM_WRENCH, false);

	sc_packet_update_room update_room_packet;
	update_room_packet.size = sizeof(update_room_packet);
	update_room_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE;
	update_room_packet.join_member = m_room_manager->Get_Room_Info(packet->room_number)->Get_Number_of_users();
	update_room_packet.room_number = packet->room_number;
	update_room_packet.state = m_room_manager->Get_Room_Info(packet->room_number)->_room_state;

	for (auto& cl : m_clients)
	{
		if (cl.get_id() == _user_id)
			continue;

		if (cl.get_state() != CLIENT_STATE::ST_LOBBY)
			continue;

		if (cl.get_look_lobby_page() != packet->room_number / 6)
			continue;

		cl.do_send(sizeof(update_room_packet), &update_room_packet);
	}
}

void cGameServer::Process_Join_Room(const int user_id, void* buff)
{
	cs_packet_join_room* packet = reinterpret_cast<cs_packet_join_room*>(buff);
	//cout << "Process_Join_Room" << endl;

	if (m_room_manager->Get_Room_Info(packet->room_number)->_room_state == GAME_ROOM_STATE::READY)
	{
		if (m_room_manager->Join_room(user_id, packet->room_number))
		{
			m_clients[user_id].set_item_own(GAME_ITEM::ITEM_DRILL, false);
			m_clients[user_id].set_item_own(GAME_ITEM::ITEM_HAMMER, false);
			m_clients[user_id].set_item_own(GAME_ITEM::ITEM_PLIERS, false);
			m_clients[user_id].set_item_own(GAME_ITEM::ITEM_WRENCH, false);

			m_clients[user_id].set_join_room_number(packet->room_number);
			m_clients[user_id]._state_lock.lock();
			m_clients[user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
			m_clients[user_id]._state_lock.unlock();
			//send_put_player_data(user_id);
			Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

			//for (int i = 0; i < 6; ++i)
			//{
			//	if (room.Get_Join_Member(i) != -1 && room.Get_Join_Member(i) != user_id)
			//	{
			//		m_clients[user_id]._room_list_lock.lock();
			//		m_clients[user_id].room_list.insert(room.Get_Join_Member(i));
			//		send_put_other_player(room.Get_Join_Member(i), user_id);
			//		m_clients[user_id]._room_list_lock.unlock();
			//	}
			//}

			// 이제 여기에 그 방에 존재하는 모든 사람에게 누가 접속했는지 정보를 전달해야함!
			send_join_room_success_packet(user_id);
			//cout << "send_join_room_success_packet" << endl;

			sc_packet_update_room update_room_packet;
			update_room_packet.size = sizeof(update_room_packet);
			update_room_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE;
			update_room_packet.join_member = m_room_manager->Get_Room_Info(packet->room_number)->Get_Number_of_users();
			update_room_packet.room_number = packet->room_number;
			m_room_manager->Get_Room_Info(packet->room_number)->_room_state_lock.lock();
			update_room_packet.state = m_room_manager->Get_Room_Info(packet->room_number)->_room_state;
			m_room_manager->Get_Room_Info(packet->room_number)->_room_state_lock.unlock();

			for (auto& cl : m_clients)
			{
				if (cl.get_id() == user_id)
					continue;

				if (cl.get_state() != CLIENT_STATE::ST_LOBBY)
					continue;

				if (cl.get_look_lobby_page() != packet->room_number / 6)
					continue;

				cl.do_send(sizeof(update_room_packet), &update_room_packet);
			}
		}
		else {
			send_join_room_fail_packet(user_id);
			//cout << "send_join_room_fail_packet" << endl;
		}
	}
	else
		send_join_room_fail_packet(user_id);

}

void cGameServer::Process_Exit_Room(const int user_id, void* buff)
{
	cs_packet_request_exit_room* packet = reinterpret_cast<cs_packet_request_exit_room*>(buff);

	int request_exit_room_number = m_clients[user_id].get_join_room_number();

	if (m_clients[user_id].get_join_room_number() != -1) {
		Room& cl_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
		cl_room.Exit_Player(user_id);
		m_clients[user_id].set_join_room_number(-1);
		m_clients[user_id]._state_lock.lock();
		m_clients[user_id].set_state(CLIENT_STATE::ST_LOBBY);
		m_clients[user_id]._state_lock.unlock();

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
			get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 10);
		}

		send_packet.size = sizeof(send_packet);
		send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
		m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
	}

	sc_packet_update_room update_room_packet;
	update_room_packet.size = sizeof(update_room_packet);
	update_room_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE;
	update_room_packet.join_member = m_room_manager->Get_Room_Info(request_exit_room_number)->Get_Number_of_users();
	update_room_packet.room_number = request_exit_room_number;
	update_room_packet.state = m_room_manager->Get_Room_Info(request_exit_room_number)->_room_state;

	for (auto& cl : m_clients)
	{
		if (cl.get_id() == user_id)
			continue;

		if (cl.get_state() != CLIENT_STATE::ST_LOBBY)
			continue;

		if (cl.get_look_lobby_page() != request_exit_room_number / 6)
			continue;

		cl.do_send(sizeof(update_room_packet), &update_room_packet);
	}
}

void cGameServer::Process_Ready(const int user_id, void* buff)
{
	cs_packet_ready* packet = reinterpret_cast<cs_packet_ready*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
#if PRINT
	cout << "id : " << user_id << "가 ready함" << endl;
#endif
	room.SetReady(packet->ready_type, user_id);
	if (room.All_Player_Ready())
	{
		for (auto put_id : room.in_player)
		{
			send_put_player_data(put_id);
			m_clients[put_id].set_bounding_box(m_clients[put_id].get_user_position(), XMFLOAT3(0.7f, 1.f, 0.7f), XMFLOAT4(0, 0, 0, 1));
			for (auto recv_id : room.in_player)
			{
				if (put_id == recv_id)
					continue;
				send_put_other_player(put_id, recv_id);
			}
		}

		for (auto player_index : room.in_player) {
			send_game_start_packet(player_index);
			m_clients[player_index]._state_lock.lock();
			m_clients[player_index].set_state(CLIENT_STATE::ST_INGAME);
			m_clients[player_index]._state_lock.unlock();
		}
		// 모든 플레이어가 레디가 된 경우 이제 게임을 시작하게 바꿔줘야하는 부분!

		sc_packet_game_start start_packet;
		start_packet.size = sizeof(start_packet);
		start_packet.type = SC_PACKET::SC_PACKET_GAME_START;
		for (auto player_id : room.in_player)
		{
			if (player_id == -1)
				continue;
			m_clients[player_id].do_send(sizeof(start_packet), &start_packet);
		}
		room.Start_Game();
	}
}

void cGameServer::Process_Game_Start(const int user_id)
{
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	room.SetLoading(true, user_id);
	if (room.All_Player_Loading())
		room.Start_Game();
}

void cGameServer::Process_Request_Room_Info(const int user_id, void* buff)
{
	cs_packet_request_all_room_info* packet = reinterpret_cast<cs_packet_request_all_room_info*>(buff);
	m_clients[user_id].set_look_lobby_page(packet->request_page);

	sc_packet_request_room_info send_packet;

	int room_number;
	for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1페이지당 10개의 방이 보인다고 가정, 룸정보를 전송할 준비함
	{
		room_number = i + (packet->request_page * MAX_ROOM_INFO_SEND);
		Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
		send_packet.room_info[i].room_number = room_number;
		send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
		send_packet.room_info[i].state = get_room_info._room_state;
		get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 10);
	}

	send_packet.size = sizeof(send_packet);
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
	DB_Request request;
	strcpy_s(request.request_char_name, sizeof(packet->id), packet->id);
	request.request_id = c_id;
	request.request_name = stringToWstring(stringID);
	request.request_pw = stringToWstring(stringPW);
	request.type = REQUEST_LOGIN;

	m_database->insert_request(request);
}

void cGameServer::Process_Voice_Data(int user_id)
{
	// 여기서 토큰을 제작해주는게 좋아보임.
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
	m_clients[user_id].m_customizing->Set_Mouthandnoses_Custom(static_cast<MOUTHANDNOSES>(packet->mouthandnoses));
 

	sc_packet_customizing_update send_packet;
	send_packet.size = sizeof(send_packet);
	send_packet.type = SC_PACKET::SC_PACKET_CUSTOMIZING;
	send_packet.id = user_id;

	send_packet.head = packet->head;
	send_packet.body = packet->body;
	send_packet.body_parts = packet->body_parts;
	send_packet.eyes = packet->eyes;
	send_packet.gloves = packet->gloves;
	send_packet.mouthandnoses = packet->mouthandnoses;


	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	for (int i = 0; i < 6; ++i)
	{
		if (rl.Get_Join_Member(i) != user_id && rl.Get_Join_Member(i) != -1)
		{
			m_clients[rl.Get_Join_Member(i)].do_send(sizeof(send_packet), &send_packet);
		}
	}
}

void cGameServer::Process_Attack(const int user_id)
{
	XMFLOAT3 attacker_look{ static_cast<float>(m_clients[user_id].get_look_x()) / 100, static_cast<float>(m_clients[user_id].get_look_y()) / 100, static_cast<float>(m_clients[user_id].get_look_z()) / 100 };
	BoundingOrientedBox punch = m_clients[user_id].get_bounding_box();

	if (!m_clients[user_id].get_user_attack_animation())
	{
		m_clients[user_id].set_attack_animation(true);

		TIMER_EVENT ev;
		ev.event_time = chrono::system_clock::now() + 499ms;
		ev.event_type = EventType::PLAYER_ATTACK;
		ev.room_number = m_clients[user_id].get_join_room_number();
		ev.obj_id = user_id;

		m_timer_queue.push(ev);
	}
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	punch.Center = Add(punch.Center, attacker_look);
	for (auto other_player_id : room.in_player)
	{
		if (punch.Intersects(m_clients[other_player_id].get_bounding_box()))
		{

			if (!m_clients[other_player_id].get_user_victim_animation())
			{
				m_clients[other_player_id].set_victim_animation(true);

				TIMER_EVENT ev;
				ev.event_time = chrono::system_clock::now() + 499ms;
				ev.event_type = EventType::PLAYER_VICTIM;
				ev.room_number = m_clients[other_player_id].get_join_room_number();
				ev.obj_id = other_player_id;

				m_timer_queue.push(ev);

				if (m_clients[other_player_id].get_life_chip())
				{
					m_clients[other_player_id].set_life_chip(false);
					send_life_chip_update(other_player_id);
				}
			}
		}
	}
}

void cGameServer::Process_Door(const int user_id, void* buff)
{
	cs_packet_request_open_door* packet = reinterpret_cast<cs_packet_request_open_door*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	bool is_door_open = room.Is_Door_Open(packet->door_num);
	room.Update_Door(static_cast<int>(packet->door_num));
	sc_packet_open_door door_packet;
	door_packet.size = sizeof(door_packet);
	door_packet.type = SC_PACKET::SC_PACKET_DOOR_UPDATE;
	door_packet.door_number = static_cast<int>(packet->door_num);
	door_packet.door_state = room.Get_Door_State(static_cast<int>(packet->door_num));

	//m_clients[user_id].do_send(sizeof(door_packet), &door_packet);
	//m_clients[user_id]._room_list_lock.lock();
	for (auto player_id : room.in_player)
		m_clients[player_id].do_send(sizeof(door_packet), &door_packet);
	//m_clients[user_id]._room_list_lock.unlock();

	if (!is_door_open)
	{
		TIMER_EVENT ev;
		ev.event_type = EventType::OPEN_DOOR;
		ev.event_time = chrono::system_clock::now() + 400ms;
		ev.obj_id = packet->door_num;
		ev.room_number = m_clients[user_id].get_join_room_number();
		m_timer_queue.push(ev);
	}
	else
	{
		TIMER_EVENT ev;
		ev.event_type = EventType::CLOSE_DOOR;
		ev.event_time = chrono::system_clock::now() + 400ms;
		ev.obj_id = packet->door_num;
		ev.room_number = m_clients[user_id].get_join_room_number();
		m_timer_queue.push(ev);
	}
}

void cGameServer::Process_ElectronicSystem_Open(const int user_id, void* buff)
{
	cs_packet_request_electronic_system_open* packet = reinterpret_cast<cs_packet_request_electronic_system_open*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	bool is_electronioc_system_door_open = room.Is_ElectronicSystem_Open(packet->es_num);
	room.Update_ElectronicSystem_Door(packet->es_num, packet->is_door_open);

	sc_packet_open_electronic_system_door es_packet;
	es_packet.size = sizeof(es_packet);
	es_packet.type = SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_DOOR_UPDATE;
	es_packet.es_num = packet->es_num;
	es_packet.es_state = room.Get_EletronicSystem_State(static_cast<int>(packet->es_num));

	//m_clients[user_id]._room_list_lock.lock();
	for (auto player_id : room.in_player)
		m_clients[player_id].do_send(sizeof(es_packet), &es_packet);
	//m_clients[user_id]._room_list_lock.unlock();

	if (!is_electronioc_system_door_open)
	{
		TIMER_EVENT ev;
		ev.event_time = chrono::system_clock::now() + 400ms;
		ev.event_type = EventType::OPEN_ELECTRONIC;
		ev.obj_id = packet->es_num;
		ev.room_number = m_clients[user_id].get_join_room_number();

		m_timer_queue.push(ev);
	}
	else
	{
		TIMER_EVENT ev;
		ev.event_time = chrono::system_clock::now() + 400ms;
		ev.event_type = EventType::CLOSE_ELECTRONIC;
		ev.obj_id = packet->es_num;
		ev.room_number = m_clients[user_id].get_join_room_number();

		m_timer_queue.push(ev);
	}
}

void cGameServer::Process_ElectronicSystem_Reset_By_Player(const int user_id, void* buff)
{
	cs_packet_request_electronic_system_reset* packet = reinterpret_cast<cs_packet_request_electronic_system_reset*>(buff);
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());


	for (int i = 0; i < ON_OFF_SWITCH; ++i)
		room.m_electrinic_system[packet->switch_index].Set_On_Off_Switch_Value(i, false);

	sc_packet_request_electronic_system_reset update_packet;
	update_packet.size = sizeof(update_packet);
	update_packet.type = SC_PACKET::SC_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_PLAYER;
	update_packet.switch_index = packet->switch_index;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		m_clients[player_id].do_send(sizeof(update_packet), &update_packet);
	}
}

void cGameServer::Process_ElectronicSystem_Reset_By_Tagger(const int user_id, void* buff)
{
	cs_packet_request_electronic_system_reset* packet = reinterpret_cast<cs_packet_request_electronic_system_reset*>(buff);
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());


	for (int i = 0; i < ON_OFF_SWITCH; ++i)
		room.m_electrinic_system[packet->switch_index].Set_On_Off_Switch_Value(i, false);

	sc_packet_request_electronic_system_reset update_packet;
	update_packet.size = sizeof(update_packet);
	update_packet.type = SC_PACKET::SC_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_TAGGER;
	update_packet.switch_index = packet->switch_index;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		m_clients[player_id].do_send(sizeof(update_packet), &update_packet);
	}
}

void cGameServer::Process_ElectronicSystem_Control(const int user_id, void* buff)
{
	cs_packet_request_eletronic_system_switch_control* packet = reinterpret_cast<cs_packet_request_eletronic_system_switch_control*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	//room.m_electrinic_system[packet->electronic_system_index].m_state_lock->lock();
	room.m_electrinic_system[packet->electronic_system_index].Set_On_Off_Switch_Value(packet->switch_idx, packet->switch_value);
	//room.m_electrinic_system[packet->electronic_system_index].m_state_lock->unlock();

	//room.m_electrinic_system[packet->electronic_system_index].Get_On_Off_Switch_Vaild();

	sc_packet_electronic_system_update_value es_packet;
	es_packet.size = sizeof(es_packet);
	es_packet.type = SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_SWITCH_UPDATE;
	es_packet.es_num = packet->electronic_system_index;
	es_packet.es_switch_idx = packet->switch_idx;
	es_packet.es_value = room.m_electrinic_system[packet->electronic_system_index].Get_On_Off_Switch_Value(packet->switch_idx);
	
	for (int i = 0; i < JOIN_ROOM_MAX_USER; ++i)
	{
		int player_id = room.in_player[i];
		m_clients[player_id].do_send(sizeof(es_packet), &es_packet);
	}
}

void cGameServer::Process_ElectronicSystem_Activate(const int user_id, void* buff)
{
	cs_packet_request_electronic_system_activate* packet = reinterpret_cast<cs_packet_request_electronic_system_activate*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	
	bool ret = room.m_electrinic_system[packet->system_index].Activate_ElectronicSystem();

	sc_packet_electronic_system_activate_update update_packet;
	update_packet.size = sizeof(update_packet);
	update_packet.type = SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_ACTIVATE_UPDATE;
	update_packet.system_index = packet->system_index;
	if (ret)
		update_packet.activate = true;
	else
		update_packet.activate = false;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		
		m_clients[player_id].do_send(sizeof(update_packet), &update_packet);
	}


	//여기서 모든 전력장치가 수리되었을 경우를 판단
	if (room.All_ElectronicSystem_Fixed())
	{
		int ativate_escape_system_index = rand() % 3;
		room.m_escape_system[ativate_escape_system_index].Activate();
		// 모든 전력장치 수리가 완료되었을 경우 탈출장치 활성화 시켜야함
	}


}

void cGameServer::Process_Item_Box_Update(const int user_id, void* buff)
{
	cs_packet_item_box_update* packet = reinterpret_cast<cs_packet_item_box_update*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	sc_packet_item_box_update update_packet;
	update_packet.size = sizeof(update_packet);
	update_packet.type = SC_PACKET::SC_PACKET_ITEM_BOX_UPDATE;
	update_packet.box_index = packet->index;
	update_packet.is_open = packet->is_open;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		m_clients[player_id].do_send(sizeof(update_packet), &update_packet);
	}
}

void cGameServer::Process_Pick_Fix_Item(const int user_id, void* buff)
{
	cs_packet_pick_fix_item* packet = reinterpret_cast<cs_packet_pick_fix_item*>(buff);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	int item_index = -1;
	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
	{
		if (packet->index == room.m_fix_item[i].Get_Item_box_index())
		{
			item_index = room.m_fix_item[i].Get_Item_box_index();
			break;
		}
	}

	if (item_index == -1)
		return;

	bool ret = room.Pick_Item(item_index);

	if (ret == false)
		return;

	if (room.m_fix_item[item_index].Get_Item_Type() == GAME_ITEM::ITEM_DRILL)
		m_clients[user_id].set_item_own(GAME_ITEM::ITEM_DRILL, true);
	else if (room.m_fix_item[item_index].Get_Item_Type() == GAME_ITEM::ITEM_HAMMER)
		m_clients[user_id].set_item_own(GAME_ITEM::ITEM_HAMMER, true);
	else if (room.m_fix_item[item_index].Get_Item_Type() == GAME_ITEM::ITEM_PLIERS)
		m_clients[user_id].set_item_own(GAME_ITEM::ITEM_PLIERS, true);
	else if (room.m_fix_item[item_index].Get_Item_Type() == GAME_ITEM::ITEM_WRENCH)
		m_clients[user_id].set_item_own(GAME_ITEM::ITEM_WRENCH, true);
	else if (room.m_fix_item[item_index].Get_Item_Type() == GAME_ITEM::ITEM_LIFECHIP)
		m_clients[user_id].set_item_own(GAME_ITEM::ITEM_LIFECHIP, true);

	sc_packet_pick_fix_item_update item_packet;
	item_packet.size = sizeof(item_packet);
	item_packet.type = SC_PACKET::SC_PACKET_PICK_ITEM_UPDATE;
	item_packet.own_id = user_id;
	item_packet.item_type = room.m_fix_item[item_index].Get_Item_Type();
	item_packet.box_index = packet->index;
	item_packet.item_show = false;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		
		m_clients[player_id].do_send(sizeof(item_packet), &item_packet);
	}
	// 이제 여기에 아이템 획득유무를 나타내고 맵에 보이는걸 비활성화 해야함
}

void cGameServer::Process_Active_Altar(const int user_id)
{
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	room.Activate_Altar();

	sc_packet_activate_altar packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_ACTIVATE_ALTAR;

	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		m_clients[player_id].do_send(sizeof(packet), &packet);
	}
}

void cGameServer::Process_Altar_LifeChip_Update(const int user_id)
{
	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	room.m_altar->Add_Life_Chip();


	sc_packet_altar_lifechip_update packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_ALTAR_LIFECHIP_UPDATE;
	packet.lifechip_count = room.m_altar->Get_Life_Chip();
	for (auto player_id : room.in_player)
	{
		if (player_id == -1)
			continue;
		m_clients[player_id].do_send(sizeof(packet), &packet);
	}
}

void cGameServer::Server_End()
{
	server_end = true;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (m_clients[i].get_state() == CLIENT_STATE::ST_FREE)
			continue;
		Disconnect(i);
	}
	//m_room_manager->Release();
}