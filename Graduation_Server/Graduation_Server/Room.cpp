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
	for (auto& p : in_player)
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
#if DEBUG
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() < 60)
		cout << "술래가 " << 60 - std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() << "초 후에 결정됩니다." << endl;
#endif // DEBUG
	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 60 && m_tagger_id == -1)
	{
		m_tagger_id = in_player[Select_Tagger()];
#if DEBUG
		cout << "술래로 player [" << m_tagger_id << "]가 선정되었습니다." << endl;
#endif // DEBUG
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count() > 600)
	{
		End_Game();
	}
	duration_time = std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count();
}

bool Room::Is_Door_Open()
{
	return true;
}

bool Room::is_collision_player_to_object(const int player_id)
{
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);

	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	for (auto& object : m_game_object)
	{
		BoundingOrientedBox object_bounding_box = object.Get_BoundingBox();
		// 여기서 충돌을 체크합니다.
		if (player_bounding_box.Intersects(object_bounding_box))
			return true;
	}
	return false;
}

bool Room::is_collision_player_to_player(const int player_id)
{
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);
	BoundingOrientedBox player_bounding_box = cl.get_bounding_box();
	for (int i = 0; i < 6; ++i)
	{
		if (in_player[i] == player_id)
			continue;

		CLIENT& other_player = *server.get_client_info(in_player[i]);
		BoundingOrientedBox other_player_bounding_box = other_player.get_bounding_box();

		if (player_bounding_box.Intersects(other_player_bounding_box))
			return true;
	}
	return false;
}

bool Room::is_collision_wall_to_player(const int player_id)
{
	cGameServer& server = cGameServer::GetInstance();
	CLIENT& cl = *server.get_client_info(player_id);

	return true;
}