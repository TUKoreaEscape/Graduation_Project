#include "Room.h"

void Room::Reset_Room()
{
	cout << "¹æ ¸®¼Â" << endl;
	room_number = -1;
	Number_of_users = 0;
	remain_user = 6;
	_room_state = GAME_ROOM_STATE::FREE;
	for (auto p : in_player)
		p = -1;
}

void Room::Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state)
{
	_room_state = room_state;
	in_player[Number_of_users] = make_player_id;
	Number_of_users++;
	remain_user = 6 - Number_of_users;
}

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

void Room::Exit_Player(int user_id)
{
	for (auto p : in_player)
	{
		if (p == user_id) {
			p = -1;
			cout << Number_of_users << "->";
			Number_of_users -= 1;
			cout << Number_of_users << endl;
			remain_user = 6 - Number_of_users; 
			if (Number_of_users == 0)
				Reset_Room();
			break;
		}
	}
}

int Room::Get_Number_of_users()
{
	int return_user_num;
	return_user_num = Number_of_users;
	return return_user_num;
}

char* Room::Get_Room_Name(char room_name[], int size)
{
	for (int i = 0; i < size; ++i)
		room_name[i] = m_room_name[i];

	return room_name;
}