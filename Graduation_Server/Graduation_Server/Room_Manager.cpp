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