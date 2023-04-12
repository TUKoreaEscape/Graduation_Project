#include "Room.h"
#include "GameServer.h"

void Room::Reset_Room()
{
	Number_of_users = 0;
	remain_user = 6;
	_room_state = GAME_ROOM_STATE::FREE;
	in_player.fill(-1);
	in_player_ready.fill(false);
	in_player_loading_success.fill(false);
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
			if (Number_of_users == 0)
				Reset_Room();
			break;
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

void Room::add_game_object(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	m_game_object.emplace_back(GameObject(ob_type, center, extents, orientation));
}

void Room::add_game_walls(Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_game_wall_and_door_and_fix_object.emplace_back(GameObject(ob_type, center, extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
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
		in_player_lock.lock();
		if (in_player[i] == user_id) {
			in_player_ready[i] = is_ready;
			in_player_lock.unlock();
			break;
		}
		in_player_lock.unlock();
	}
}

void Room::SetLoading(const bool is_loading, const int user_id)
{
	for (int i = 0; i < in_player_loading_success.size(); ++i)
	{
		in_player_lock.lock();
		if (in_player[i] == user_id) {
			in_player_loading_success[i] = is_loading;
			in_player_lock.unlock();
			break;
		}
		in_player_lock.unlock();
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

int Room::Select_Tagger()
{
	mt19937 engine((unsigned int)time(NULL));

	return engine() % 6;
}

void Room::Start_Game()
{
	start_time = chrono::system_clock::now();
	_room_state = GAME_ROOM_STATE::PLAYING;

	cGameServer& server = cGameServer::GetInstance();

	for (auto p : in_player)
	{
		CLIENT& player = *server.get_client_info(p);
		player.set_user_position({ 100,100,100 });
	}
}

void Room::End_Game()
{
	_room_state_lock.lock();
	_room_state = GAME_ROOM_STATE::END;
	_room_state_lock.unlock();

	cout << "게임이 종료되었습니다." << endl;
	// 엔딩패킷처리 보내는곳

}

void Room::Update_room_time()
{
	now_time = chrono::system_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 60 && m_tagger_id == -1)
	{
		m_tagger_id = in_player[Select_Tagger()];
		cGameServer& server = cGameServer::GetInstance();
		CLIENT& cl = *server.get_client_info(m_tagger_id);

		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}

	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(m_tagger_id);

	
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 180 && false == m_first_skill_enable) // 술래 첫번째 스킬 활성화
	{
		m_first_skill_enable = true;
		cl.set_first_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 360 && false == m_second_skill_enable) // 술래 두번째 스킬 활성화
	{
		m_second_skill_enable = true;
		cl.set_second_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
	}
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 540 && false == m_third_skill_enable) // 술래 세번째 스킬 활성화
	{
		m_third_skill_enable = true;
		cl.set_third_skill_enable();
		sc_packet_tagger_skill packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET::SC_PACKET_TAGGER_SKILL;
		packet.first_skill = m_first_skill_enable;
		packet.second_skill = m_second_skill_enable;
		packet.third_skill = m_third_skill_enable;
		cl.do_send(sizeof(packet), &packet);
	}


	// 게임 종료를 확인하는 부분
	if (m_tagger_collect_chip == GAME_END_COLLECT_CHIP) // 술래가 정해진 갯수의 생명칩을 수거한 경우
	{
		End_Game();
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 720) // 게임종료 확인(타임아웃)
	{
		End_Game();
	}
	duration_time = std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count();
}

void Room::Update_Player_Position()
{
	cGameServer& server = cGameServer::GetInstance();
}

bool Room::Is_Door_Open()
{
	return true;
}

CollisionInfo Room::is_collision_player_to_object(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift)
{
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = cl.get_bounding_box();
	for (auto& object : m_game_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
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

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.CollisionNormal = collision_data.CollisionNormal;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			cl.set_user_position(current_position);
			cl.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}
	return return_data;
}

CollisionInfo Room::is_collision_player_to_player(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift)
{
	CollisionInfo return_data;

	int collied_id = -1;
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);

	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	for (int i = 0; i < Number_of_users; ++i)
	{
		if (in_player[i] != player_id && in_player[i] != -1)
		{
			CLIENT& other_player = *server.get_client_info(in_player[i]);
			BoundingOrientedBox other_player_bounding_box = other_player.get_bounding_box();

			if (player_bounding_box.Intersects(other_player_bounding_box)) {
				collied_id = in_player[i];
				break;
			}
		}
	}

	if (collied_id != -1)
	{
		XMFLOAT3 SlidingVector = XMFLOAT3(0.0f, 0.0f, 0.0f);
		cl.set_user_position(current_position);
		cl.update_bounding_box_pos(current_position);

		CLIENT& other = *server.get_client_info(collied_id);
		player_bounding_box = cl.get_bounding_box();
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
	}
	else
	{
		return_data.is_collision = false;
		return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
		return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	}
	return return_data;
}

CollisionInfo Room::is_collision_wall_to_player(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift)
{
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);
	CollisionInfo return_data;
	return_data.is_collision = false;
	return_data.CollisionNormal = XMFLOAT3(0, 0, 0);
	return_data.SlidingVector = XMFLOAT3(0, 0, 0);
	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	XMFLOAT3 MotionVector = xmf3shift;
	XMFLOAT3 tmp_position = current_position;
	BoundingOrientedBox check_box = cl.get_bounding_box();
	for (auto& object : m_game_wall_and_door_and_fix_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
	{
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

			return_data.is_collision = true;
			return_data.SlidingVector = SlidingVector;
			return_data.CollisionNormal = collision_data.CollisionNormal;
			MotionVector = SlidingVector;
			tmp_position = Add(tmp_position, SlidingVector);

			cl.set_user_position(current_position);
			cl.update_bounding_box_pos(current_position);
			check_box.Center = tmp_position;
		}
	}
	return return_data;
}


