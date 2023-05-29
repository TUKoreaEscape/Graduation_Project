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
	m_game_wall_and_fix_object.emplace_back(GameObject(ob_type, center, extents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
}

void Room::add_game_doors(const unsigned int door_id, Object_Type ob_type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_door_object.emplace_back(Door());
	Door& this_door = *m_door_object.end();
	this_door.init_data(door_id, ob_type, center, BoundingOrientedBox{ center, extents, XMFLOAT4{0,0,0,1} });
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

void Room::Update_Player_Position() // 사용하지 않습니다.
{
	cGameServer& server = cGameServer::GetInstance();
}

void Room::Update_Door(const int door_num)
{
	m_door_object[door_num].process_door_event();
}

bool Room::Is_Door_Open(const int door_num)
{
	if(m_door_object[door_num].get_state() == ST_OPEN)
		return true;
	return false;
}

CollisionInfo Room::is_collision_player_to_object(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift)
{
	cGameServer& server = cGameServer::GetInstance();
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

CollisionInfo Room::is_collision_player_to_player(const int player_id, const XMFLOAT3 current_position, const XMFLOAT3 xmf3shift)
{
	CollisionInfo return_data;

	int collied_id = -1;
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& client = *server.get_client_info(player_id);

	BoundingOrientedBox player_bounding_box = client.get_bounding_box();
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
	int collied_face_num = -1;
	for (auto& object : m_game_wall_and_fix_object) // 모든벽을 체크 후 값을 더해주는 방식이 좋아보임!
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


