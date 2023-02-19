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