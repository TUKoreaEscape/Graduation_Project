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

void RoomManager::Create_room()
{
	in_game_room.emplace_back(Room());	// �������� ���� �߰��Ҷ� ������� ��� �������! 
}