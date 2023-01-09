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

void RoomManager::Create_room(int user_id)
{
	in_game_room.emplace_back(Room(user_id));	// 아직까진 방을 추가할때 멤버변수 없어서 비어있음! 
}