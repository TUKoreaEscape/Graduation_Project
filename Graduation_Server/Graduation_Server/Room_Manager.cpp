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
}

void RoomManager::Create_room(int user_id)
{
	in_game_room.emplace_back(Room(user_id));	// �������� ���� �߰��Ҷ� ������� ��� �������! 
}

bool RoomManager::collision_player_player(int user_id)
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id)
{
	return true;
}