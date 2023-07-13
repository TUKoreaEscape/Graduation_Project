#include "Room.h"
#include "GameServer.h"

void Room::init_room_name(int rand_num)
{
	switch (rand_num) {
	case 0:
		strcpy_s(m_room_name, sizeof(room_name1), room_name1);
		break;
	case 1:
		strcpy_s(m_room_name, sizeof(room_name2), room_name2);
		break;
	case 2:
		strcpy_s(m_room_name, sizeof(room_name3), room_name3);
		break;
	case 3:
		strcpy_s(m_room_name, sizeof(room_name4), room_name4);
		break;
	case 4:
		strcpy_s(m_room_name, sizeof(room_name5), room_name5);
		break;
	case 5:
		strcpy_s(m_room_name, sizeof(room_name6), room_name6);
		break;
	case 6:
		strcpy_s(m_room_name, sizeof(room_name7), room_name7);
		break;
	}
}

void Room::Reset_Room()
{
	Number_of_users = 0;
	remain_user = 6;
	_room_state = GAME_ROOM_STATE::FREE;
	in_player.fill(-1);
	in_player_ready.fill(false);
	in_player_loading_success.fill(false);

	for (int i = 0; i < m_door_object.size(); ++i)
		m_door_object[i].m_check_bounding_box = true;

	for (int i = 0; i < m_electrinic_system.size(); ++i)
		m_electrinic_system[i].Reset();

	for (int i = 0; i < m_escape_system.size(); ++i)
		m_escape_system[i].init();

	for (int i = 0; i < m_vent_object.size(); ++i)
		m_vent_object[i].init();

	m_first_skill_enable = false;
	m_second_skill_enable = false;
	m_third_skill_enable = false;

	m_tagger_id = -1;
	m_tagger_collect_chip = 0;
	m_altar->init();
}

void Room::init_room_by_game_end()
{
	in_player_ready.fill(false);
	in_player_loading_success.fill(false);

	for (int i = 0; i < m_door_object.size(); ++i)
		m_door_object[i].init();

	for (int i = 0; i < m_electrinic_system.size(); ++i)
		m_electrinic_system[i].Reset();

	for (int i = 0; i < m_escape_system.size(); ++i)
		m_escape_system[i].init();

	for (int i = 0; i < m_vent_object.size(); ++i)
		m_vent_object[i].init();

	m_first_skill_enable = false;
	m_second_skill_enable = false;
	m_third_skill_enable = false;

	m_tagger_id = -1;
	m_tagger_collect_chip = 0;
	m_altar->init();

	cGameServer& server = *cGameServer::GetInstance();
	int i = 0;
	for (int& player_id : in_player)
	{
		if (player_id == -1)
			continue;
		server.m_clients[player_id].set_user_position(XMFLOAT3(static_cast<float>(6.f - ((float)i * 2.5)), 5.f, -10.f));
		server.m_clients[player_id].set_life_chip(false);
		i++;
	}
}

void Room::Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state) // <- 방 만들때 in_player가 전부 -1에서 0으로 바뀜 이거 수정해야댐
{
	in_player.fill(-1);
	in_player_ready.fill(false);
	in_player_loading_success.fill(false);
	_room_state = room_state;
	in_player[Number_of_users] = make_player_id;
	Number_of_users++;
	remain_user = 6 - Number_of_users;
}

bool Room::Join_Player(int user_id)
{
	for (auto &p : in_player)
	{
		if (p == -1) {
			p = user_id;
			Number_of_users++;
			remain_user = 6 - Number_of_users;
			return true;
		}
	}
	return false;
}

void Room::Exit_Player(int user_id)
{
	for (int i = 0; i < in_player.size(); ++i)
	{
		if (in_player[i] == user_id)
		{
			in_player[i] = -1;
			in_player_ready[i] = false;
			in_player_loading_success[i] = false;
			Number_of_users -= 1;
			remain_user = 6 - Number_of_users;

			if (Number_of_users == 0) {
				Reset_Room();
			}
			break;
		}
	}

	if (Number_of_users > 0) {
		sc_packet_player_exit packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_PLAYER_EXIT;
		packet.user_id = user_id;


		cGameServer& server = *cGameServer::GetInstance();
		for (int& player_id : in_player)
		{
			if (player_id == -1)
				continue;
			server.m_clients[player_id].do_send(sizeof(packet), &packet);
		}
	}
}

int Room::Get_Number_of_users()
{
	int return_user_num;
	return_user_num = Number_of_users;
	return return_user_num;
}

int Room::Get_Join_Member(int data)
{
	return in_player[data];
}

char* Room::Get_Room_Name(char room_name[], int size)
{
	strcpy_s(room_name, size, m_room_name);
	return room_name;
}

void Room::init_fix_object_and_life_chip()
{
	int temp[MAX_INGAME_ITEM];
	srand(time(NULL));
	for (int i = 0; i < MAX_INGAME_ITEM; ++i) {
		temp[i] = -1;
		m_fix_item[i].init();
	}

	for (int i = 0; i < MAX_INGAME_ITEM; ++i) {
		int t = rand() % MAX_INGAME_ITEM;
		int* p;
		while (true) {
			p = find(temp, temp + MAX_INGAME_ITEM, t);
			if (p == temp + MAX_INGAME_ITEM)
			{
				m_fix_item[i].Set_Item_box_index(t);
				temp[i] = t;
				break;
			}
			else {
				t += 1;
				if (t == MAX_INGAME_ITEM)
					t = 0;
			}
		}
	}

	m_fix_item[0].Set_Item_Type(GAME_ITEM::ITEM_DRILL);
	m_fix_item[1].Set_Item_Type(GAME_ITEM::ITEM_HAMMER);
	m_fix_item[MAX_INGAME_ITEM - 1].Set_Item_Type(GAME_ITEM::ITEM_PLIERS);
	m_fix_item[MAX_INGAME_ITEM - 2].Set_Item_Type(GAME_ITEM::ITEM_WRENCH);
	m_fix_item[MAX_INGAME_ITEM - 3].Set_Item_Type(GAME_ITEM::ITEM_DRIVER);

	for (int i = 7; i < MAX_INGAME_ITEM - 3; ++i)
		m_fix_item[i].Set_Item_Type(GAME_ITEM::ITEM_LIFECHIP);

	sc_packet_pick_item_init item_init_packet;
	item_init_packet.size = sizeof(item_init_packet);
	item_init_packet.type = SC_PACKET::SC_PACKET_PICK_ITEM_INIT;
	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
	{
		item_init_packet.data[i].item_box_index = m_fix_item[i].Get_Item_box_index();
		item_init_packet.data[i].item_type = m_fix_item[i].Get_Item_Type();
	}

#if PRINT
	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
	{
		cout << "Item [" << item_init_packet.data[i].item_box_index << "] Type : ";
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_LIFECHIP)
			std::cout << "ITEM_LIFECHIP" << std::endl;
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_DRILL)
			std::cout << "ITEM_DRILL" << std::endl;
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_HAMMER)
			std::cout << "ITEM_HAMMER" << std::endl;
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_NONE)
			std::cout << "ITEM_NONE" << std::endl;
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_PLIERS)
			std::cout << "ITEM_PLIERS" << std::endl;
		if (item_init_packet.data[i].item_type == GAME_ITEM::ITEM_WRENCH)
			std::cout << "ITEM_WRENCH" << std::endl;
	}
#endif

	for (int& player_id : in_player) {
		if (player_id == -1)
			continue;
		cGameServer::GetInstance()->m_clients[player_id].do_send(sizeof(item_init_packet), &item_init_packet);
	}
}

void Room::add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	m_game_object.emplace_back(GameObject(ob_type, center, extents, orientation));
}

void Room::add_game_walls(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_game_wall.emplace_back(GameObject(ob_type, center, extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
}

void Room::add_fix_objects(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_game_fix_object.emplace_back(GameObject(ob_type, center, extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
}

void Room::add_game_doors(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_door_object.emplace_back(Door(door_id, ob_type, center, extents));
}

void Room::add_game_vents(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_vent_object.emplace_back(Vent(door_id, ob_type, center, extents));
}

void Room::add_game_ElectronicSystem(const unsigned int id, Object_Type ob_type, XMFLOAT3& center, XMFLOAT3& extents)
{
	m_electrinic_system.emplace_back(ElectronicSystem(id, ob_type, center, extents));
}

void Room::SetBoundingBox(XMFLOAT3 pos, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	for (int i = 0; i < in_player_bounding_box.size(); ++i)
	{
		in_player_bounding_box[i] = BoundingOrientedBox{ pos, extents, orientation };
	}
}

void Room::SetReady(const bool is_ready, const int user_id)
{
	for (int i = 0; i < in_player.size(); ++i)
	{
		if (in_player[i] == user_id) {
			in_player_lock.lock();
			in_player_ready[i] = is_ready;
			in_player_lock.unlock();
			break;
		}
	}
}

void Room::SetLoading(const bool is_loading, const int user_id)
{
	for (int i = 0; i < in_player_loading_success.size(); ++i)
	{
		if (in_player[i] == user_id) {
			in_player_lock.lock();
			in_player_loading_success[i] = is_loading;
			in_player_lock.unlock();
			break;
		}
	}
}

void Room::Activate_Altar()
{
	m_altar->Set_Valid(true);
}

bool Room::Is_Tagger_Winner()
{
	if (m_altar->Get_Life_Chip() == GAME_END_COLLECT_CHIP)
		return true;
	return false;
}

void Room::Set_Electronic_System_ONOFF()
{
	mt19937 engine((unsigned int)time(NULL));
	for (int i = 0; i < NUMBER_OF_ELECTRONIC; ++i)
	{
		for (int idx = 0; idx < ON_OFF_SWITCH; ++idx)
			m_electrinic_system[i].init_electrinic_switch_data(idx, (engine() % 2 + rand() % 2) % 2);
	}
}

bool Room::All_Player_Ready()
{
	for (auto p : in_player_ready)
	{
		if (p == false)
			return false;
	}
	return true;
}

bool Room::All_Player_Loading()
{
	for (auto p : in_player_loading_success)
	{
		if (p == false)
			return false;
	}
	return true;
}

void Room::Tagger_Use_First_Skill()
{
	cGameServer& server = *cGameServer::GetInstance();

	for (int i = 0; i < m_electrinic_system.size(); ++i)
		m_electrinic_system[i].Set_Close_Electronic_System();

	sc_packet_use_first_tagger_skill packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_USE_FIRST_TAGGER_SKILL;
	for (int i = 0; i < m_electrinic_system.size(); ++i) {
		if (m_electrinic_system[i].Get_Fixed_ElectronicSystem() == true)
			packet.electronic_system_close[i] = false;
		else
			packet.electronic_system_close[i] = true;
	}

	for (int& player_id : in_player) {
		if (player_id == -1)
			continue;
		server.m_clients[player_id].do_send(sizeof(packet), &packet);
	}
}

void Room::Tagger_Use_Second_Skill(int room_number)
{
	cGameServer& server = *cGameServer::GetInstance();

	TIMER_EVENT ev;
	ev.room_number = room_number;
	ev.event_type = EventType::USE_SECOND_TAGGER_SKILL;
	ev.event_time = chrono::system_clock::now() + 30s;

	server.m_timer_queue.push(ev);

	sc_packet_use_second_tagger_skill packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_USE_SECOND_TAGGER_SKILL;
	packet.is_start = true;

	for (int& player_id : in_player) {
		if (player_id == -1)
			continue;
		server.m_clients[player_id].do_send(sizeof(packet), &packet);
	}
}

void Room::Tagger_Use_Third_Skill()
{
	srand(time(NULL));
	
	cGameServer& server = *cGameServer::GetInstance();

	int unactive_vent_number = rand() % m_vent_object.size();

	sc_packet_use_third_tagger_skill packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_USE_THIRD_TAGGER_SKILL;
	packet.unactivate_vent = unactive_vent_number;

	for (int& player_id : in_player) {
		if (player_id == -1)
			continue;
		server.m_clients[player_id].do_send(sizeof(packet), &packet);
	}
}

void Room::Update_Player_Position() // 사용하지 않습니다.
{
	cGameServer& server = *cGameServer::GetInstance();
}

void Room::Update_Door(const int door_num)
{
	m_door_object[door_num].process_door_event();
}

void Room::Update_ElectronicSystem_Door(const int es_num, bool value)
{
	m_electrinic_system[es_num].Update_Object(value);
}

bool Room::Is_Door_Open(const int door_num)
{
	if(m_door_object[door_num].get_state() == ST_OPEN)
		return true;
	return false;
}

bool Room::Pick_Item(const int box_id)
{
	return m_fix_item[box_id].Pict_Item();
}

bool Room::Is_ElectronicSystem_Open(const int es_num)
{
	if (m_electrinic_system[es_num].get_state() == ES_OPEN)
		return true;
	return false;
}

CollisionInfo Room::is_collision_player_to_door(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& client = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = client.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = client.get_bounding_box();
	for (auto& object : m_door_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
		if (!object.m_check_bounding_box)
			continue;
		if (check_box.Intersects(object.Get_BoundingBox()))
		{
			client.set_user_position(tmp_position);
			client.update_bounding_box_pos(tmp_position);
			CollisionInfo collision_data = server.GetCollisionInfo(object.Get_BoundingBox(), player_bounding_box);
			XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 current_player_position = tmp_position;


			float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&collision_data.CollisionNormal)));

			if (DotProduct < 0.0f)
			{
				XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * collision_data.CollisionNormal.x, -DotProduct * collision_data.CollisionNormal.y, -DotProduct * collision_data.CollisionNormal.z);
				SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
			}
			else
			{
				SlidingVector = MotionVector;
			}

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.CollisionNormal = collision_data.CollisionNormal;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			client.set_user_position(current_position);
			client.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}
	return return_data;
}

CollisionInfo Room::is_collision_player_to_vent(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& client = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = client.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = client.get_bounding_box();
	

	for (auto& object : m_vent_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
		if (false == Is_near(current_position, object.Get_center(), 15))
			continue;

		if (!object.m_check_bounding_box) {
			if (player_id != m_tagger_id)
				continue;
		}

		if (check_box.Intersects(object.Get_BoundingBox()))
		{
			client.set_user_position(tmp_position);
			client.update_bounding_box_pos(tmp_position);
			CollisionInfo collision_data = server.GetCollisionInfo(object.Get_BoundingBox(), player_bounding_box);
			XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 current_player_position = tmp_position;


			float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&collision_data.CollisionNormal)));

			if (DotProduct < 0.0f)
			{
				XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * collision_data.CollisionNormal.x, -DotProduct * collision_data.CollisionNormal.y, -DotProduct * collision_data.CollisionNormal.z);
				SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
			}
			else
			{
				SlidingVector = MotionVector;
			}

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.CollisionNormal = collision_data.CollisionNormal;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			client.set_user_position(current_position);
			client.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}
	return return_data;
}

CollisionInfo Room::is_collision_player_to_object(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& client = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = client.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = client.get_bounding_box();
	for (auto& object : m_game_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
		if (check_box.Intersects(object.Get_BoundingBox()))
		{
			client.set_user_position(tmp_position);
			client.update_bounding_box_pos(tmp_position);
			CollisionInfo collision_data = server.GetCollisionInfo(object.Get_BoundingBox(), player_bounding_box);
			XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 current_player_position = tmp_position;


			float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&collision_data.CollisionNormal)));

			if (DotProduct < 0.0f)
			{
				XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * collision_data.CollisionNormal.x, -DotProduct * collision_data.CollisionNormal.y, -DotProduct * collision_data.CollisionNormal.z);
				SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
			}
			else
			{
				SlidingVector = MotionVector;
			}

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.CollisionNormal = collision_data.CollisionNormal;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			client.set_user_position(current_position);
			client.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}
	return return_data;
}

CollisionInfo Room::is_collision_player_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	CollisionInfo return_data;

	int collied_id = -1;
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& client = *server.get_client_info(player_id);

	BoundingOrientedBox player_bounding_box = client.get_bounding_box();
	for (int i = 0; i < Number_of_users; ++i)
	{
		if (in_player[i] == player_id)
			continue;
		if (in_player[i] == -1)
			continue;

		CLIENT& other_player = *server.get_client_info(in_player[i]);
		BoundingOrientedBox other_player_bounding_box = other_player.get_bounding_box();

		if (player_bounding_box.Intersects(other_player_bounding_box)) {
			collied_id = in_player[i];
			break;
		}
	}

	if (collied_id != -1)
	{
		XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
		client.set_user_position(current_position);
		client.update_bounding_box_pos(current_position);

		CLIENT& other = *server.get_client_info(collied_id);
		player_bounding_box = client.get_bounding_box();
		BoundingOrientedBox other_player_bounding_box = other.get_bounding_box();
		CollisionInfo data = server.GetCollisionInfo(other_player_bounding_box, player_bounding_box);

		XMFLOAT3 MotionVector = xmf3shift;
		float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&data.CollisionNormal)));
		if (DotProduct < 0.0f)
		{
			XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * data.CollisionNormal.x, -DotProduct * data.CollisionNormal.y, -DotProduct * data.CollisionNormal.z);
			SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
		}
		else
		{
			SlidingVector = MotionVector;
		}
		return_data.is_collision = true;
		return_data.SlidingVector = SlidingVector;
		return_data.CollisionNormal = data.CollisionNormal;
		return_data.collision_face_num = data.collision_face_num;
	}
	else
	{
		return_data.is_collision = false;
		return_data.collision_face_num = -1;
		return_data.SlidingVector = XMFLOAT3(0, 0, 0);
		return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	}
	return return_data;
}

CollisionInfo Room::is_collision_wall_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = cl.get_bounding_box();
	int collied_face_num = -1;
	for (auto& object : m_game_wall) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
		if (false == Is_near(current_position, object.Get_center(), 100))
			continue;
		if (check_box.Intersects(object.Get_BoundingBox()))
		{
			cl.set_user_position(tmp_position);
			cl.update_bounding_box_pos(tmp_position);
			CollisionInfo collision_data = server.GetCollisionInfo(object.Get_BoundingBox(), player_bounding_box);
			XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 current_player_position = tmp_position;


			float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&collision_data.CollisionNormal)));

			if (DotProduct < 0.0f)
			{
				XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * collision_data.CollisionNormal.x, -DotProduct * collision_data.CollisionNormal.y, -DotProduct * collision_data.CollisionNormal.z);
				SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
			}
			else
			{
				SlidingVector = MotionVector;
			}

			if (collision_data.collision_face_num == 4)
				collied_face_num = 4;

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.collision_face_num = collision_data.collision_face_num;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			cl.set_user_position(current_position);
			cl.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}

	return_data.collision_face_num = collied_face_num;
	return return_data;
}

CollisionInfo Room::is_collision_fix_object_to_player(const int& player_id, const XMFLOAT3& current_position, const XMFLOAT3& xmf3shift)
{
	cGameServer& server = *cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = cl.get_bounding_box();
	int collied_face_num = -1;
	for (auto& object : m_game_fix_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
		if (false == Is_near(current_position, object.Get_center(), 15))
			continue;
		if (check_box.Intersects(object.Get_BoundingBox()))
		{
			cl.set_user_position(tmp_position);
			cl.update_bounding_box_pos(tmp_position);
			CollisionInfo collision_data = server.GetCollisionInfo(object.Get_BoundingBox(), player_bounding_box);
			XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 current_player_position = tmp_position;


			float DotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&MotionVector), XMLoadFloat3(&collision_data.CollisionNormal)));

			if (DotProduct < 0.0f)
			{
				XMFLOAT3 RejectionVector = XMFLOAT3(-DotProduct * collision_data.CollisionNormal.x, -DotProduct * collision_data.CollisionNormal.y, -DotProduct * collision_data.CollisionNormal.z);
				SlidingVector = XMFLOAT3(MotionVector.x + RejectionVector.x, MotionVector.y + RejectionVector.y, MotionVector.z + RejectionVector.z);
			}
			else
			{
				SlidingVector = MotionVector;
			}

			if (collision_data.collision_face_num == 4)
				collied_face_num = 4;

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.collision_face_num = collision_data.collision_face_num;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			cl.set_user_position(current_position);
			cl.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}

	return_data.collision_face_num = collied_face_num;
	return return_data;
}

bool Room::Is_near(XMFLOAT3 player_pos, XMFLOAT3 object_pos, int range)
{
	if (range < abs(player_pos.x - object_pos.x)) return false;
	if (range < abs(player_pos.z - object_pos.z)) return false;
	return true;
}