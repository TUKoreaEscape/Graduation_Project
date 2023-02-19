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
}

int RoomManager::Create_room(int user_id)
{
	in_game_room.emplace_back(Room(user_id, in_game_room.size(), GAME_ROOM_STATE::READY)); // 방을 생성한 사람 id, 게임룸이 몆번방, 게임방 상태
	return in_game_room.size();
}

void RoomManager::Join_room(int user_id, int select_room_number)
{
	in_game_room[select_room_number].Join_Player(user_id);
}

bool RoomManager::collision_player_player(int user_id)
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id)
{
	return true;
}