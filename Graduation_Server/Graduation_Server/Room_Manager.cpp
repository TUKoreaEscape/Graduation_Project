#include "Room_Manager.h"

RoomManager::RoomManager()
{

}

RoomManager::~RoomManager()
{

}

void RoomManager::init()
{
	// 서버 시작시 게임룸 초기화 장소
	for (int i = 0; i < a_in_game_room.size(); ++i) {
		a_in_game_room[i]._room_state = GAME_ROOM_STATE::FREE;
		a_in_game_room[i].init_room_number(i);
	}
}

void RoomManager::init_object()
{
	ifstream in("GameObject\\object_data.txt");
	if (!in)
	{
		std::cout << "object file read fail!" << std::endl;
	}
	int type;
	XMFLOAT3 center;
	XMFLOAT3 extents;
	XMFLOAT4 orientation;

	while (in)
	{
		in >> type >> center.x >> center.y >> center.z >> extents.x >> extents.y >> extents.z >> orientation.x >> orientation.y >> orientation.z >> orientation.w;
		m_game_object.emplace_back(static_cast<Object_Type>(type), center, extents, orientation);
	}
	std::cout << "Game Object load success!!!" << std::endl;

	for (auto& p : m_game_object)
	{
		for (auto& _room : a_in_game_room)
			_room.add_game_object(p.Get_Object_Type(), p.Get_center(), p.Get_extents(), p.Get_orientation());
	}
	cout << "All room object init!!!" << endl;
}

int RoomManager::Create_room(int user_id)
{
	int return_create_room_number = -1;
	for (int i = 0; i < a_in_game_room.size(); ++i)
	{ 

		if (a_in_game_room[i]._room_state == GAME_ROOM_STATE::FREE)
		{
			a_in_game_room[i].Create_Room(user_id, i, GAME_ROOM_STATE::READY);
			return_create_room_number = i;
			return return_create_room_number;
		}
	}

	return return_create_room_number;
}

void RoomManager::Clean_room(int room_number)
{
	a_in_game_room[room_number]._room_state = GAME_ROOM_STATE::FREE;
	a_in_game_room[room_number].Reset_Room();
}

bool RoomManager::Join_room(int user_id, int select_room_number)
{
	if (a_in_game_room[select_room_number].Join_Player(user_id))
	{
		return true;
	}
	return false;
}

bool RoomManager::collision_player_player(int user_id)
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id)
{
	return true;
}

Room* RoomManager::Get_Room_Info(int room_number)
{
	return &a_in_game_room[room_number];
}