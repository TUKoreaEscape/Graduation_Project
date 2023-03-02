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

int RoomManager::Create_room(int user_id)
{
	//in_game_room.emplace_back(Room(user_id, in_game_room.size(), GAME_ROOM_STATE::READY)); // 방을 생성한 사람 id, 게임룸이 몆번방, 게임방 상태
	int return_create_room_number = -1;
	for (int i = 0; i < a_in_game_room.size(); ++i)
	{

		if (a_in_game_room[i]._room_state == GAME_ROOM_STATE::FREE)
		{
			a_in_game_room[i].Create_Room(user_id, i, GAME_ROOM_STATE::READY);
			return_create_room_number = i;
			cout << "생성된 방번호 : " << return_create_room_number << endl;
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
	cout << "room_manager join room" << endl;
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