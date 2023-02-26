#include "Room.h"

void Room::Join_Player(int user_id)
{
	for (auto p : in_player)
	{
		if (p == -1) {
			p = user_id;
			Number_of_users++;
			remain_user = 6 - Number_of_users;
			break;
		}
	}
}

int Room::Get_Number_of_users()
{
	return Number_of_users;
}

char* Room::Get_Room_Name(char room_name[], int size)
{
	for (int i = 0; i < size; ++i)
		room_name[i] = m_room_name[i];

	return room_name;
}