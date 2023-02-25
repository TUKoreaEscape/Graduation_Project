#include "Room_Manager.h"

RoomManager::RoomManager()
{

}

RoomManager::~RoomManager()
{

}

void RoomManager::init()
{
	// ���� ���۽� ���ӷ� �ʱ�ȭ ���
	for (int i = 0; i < a_in_game_room.size(); ++i)
		a_in_game_room[i]._room_state = GAME_ROOM_STATE::FREE;
}

int RoomManager::Create_room(int user_id)
{
	//in_game_room.emplace_back(Room(user_id, in_game_room.size(), GAME_ROOM_STATE::READY)); // ���� ������ ��� id, ���ӷ��� �p����, ���ӹ� ����
	int return_create_room_number;
	for (int i = 0; i < a_in_game_room.size(); ++i)
	{
		if (a_in_game_room[i]._room_state == GAME_ROOM_STATE::FREE)
		{
			a_in_game_room[i].Create_Room(user_id, i, GAME_ROOM_STATE::READY);
			return_create_room_number = i;
			break;
		}
	}
	return return_create_room_number;
}

void RoomManager::Join_room(int user_id, int select_room_number)
{
	a_in_game_room[select_room_number].Join_Player(user_id);	
}

bool RoomManager::collision_player_player(int user_id)
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id)
{
	return true;
}