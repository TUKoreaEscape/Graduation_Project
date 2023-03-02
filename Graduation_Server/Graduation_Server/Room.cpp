#include "Room.h"

void Room::Reset_Room()
{
	cout << "방 리셋" << endl;
	Number_of_users = 0;
	remain_user = 6;
	_room_state = GAME_ROOM_STATE::FREE;
	in_player.fill(-1);
}

void Room::Create_Room(int make_player_id, int room_num, GAME_ROOM_STATE::TYPE room_state) // <- 방 만들때 in_player가 전부 -1에서 0으로 바뀜 이거 수정해야댐
{
	cout << "불린횟수가?" << endl;
	for (auto p : in_player)
		cout << p << endl;
	_room_state = room_state;
	in_player[Number_of_users] = make_player_id;
	Number_of_users++;
	remain_user = 6 - Number_of_users;

	for (auto p : in_player)
		cout << p << endl;
}

bool Room::Join_Player(int user_id)
{
	for (auto &p : in_player)
	{
		cout << p << endl;
		if (p == -1) {
			p = user_id;
			Number_of_users++;
			remain_user = 6 - Number_of_users;
			cout << p << endl;

			return true;
		}
	}
	return false;
}

void Room::Exit_Player(int user_id)
{
	for (auto p : in_player)
	{
		cout << p << endl;
		if (p == user_id) {
			p = -1;
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