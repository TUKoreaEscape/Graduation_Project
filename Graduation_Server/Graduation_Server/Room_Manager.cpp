#include "Room_Manager.h"

RoomManager::RoomManager()
{

}

RoomManager::~RoomManager()
{

}

void RoomManager::init() // ���� ���� �� �ʱ�ȭ���ִ� �Լ��Դϴ�.
{
	// ���� ���۽� ���ӷ� �ʱ�ȭ ���
	for (int i = 0; i < a_in_game_room.size(); ++i) {
		a_in_game_room[i]._room_state = GAME_ROOM_STATE::FREE;
		a_in_game_room[i].init_room_number(i);
	}
}

void RoomManager::init_object() // �ʿ� ��ġ�� ������Ʈ�� �ε��ؾ��ϴ°��Դϴ�. (��������� �κ�)
{
	// ���� �ʿ� �����ϴ� ��, ������ ������Ʈ�� �浹 ������ ������ �ε��ϴ� �����Դϴ�.
	FILE* pFile = nullptr;
	fopen_s(&pFile, "walls/FixedObjectsBounding.bin", "rb");
	if (pFile)
		rewind(pFile);

	unsigned int nReads = 0;
	unsigned char nStrLength = 0;
	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };

	for (int i = 0; i < 48; ++i) 
	{

		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrToken, sizeof(char), nStrLength, pFile); // <Wall>:
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrGameObjectName, sizeof(char), nStrLength, pFile);

		pstrGameObjectName[nStrLength] = '\0';

		float AABBCenter[3]{};
		float AABBExtents[3]{};

		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(pstrToken, sizeof(char), nStrLength, pFile); //"<BoundingBox>:"
		nReads = (unsigned int)fread(AABBCenter, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBExtents, sizeof(float), 3, pFile);

		for (auto& _room : a_in_game_room) {
			_room.add_game_walls(Object_Type::OB_WALL, XMFLOAT3(AABBCenter[0], AABBCenter[1], AABBCenter[2]), XMFLOAT3(AABBExtents[0], AABBExtents[1], AABBExtents[2]));
			//cout << "Wall - " << i + 1 << " - " << pstrGameObjectName << " Center - (" << AABBCenter[0] << ", " << AABBCenter[1] << ", " << AABBCenter[2] << "), Extents - (" << AABBExtents[0] << ", " << AABBExtents[1] << ", " << AABBExtents[2] << ")" << endl;

		}
	}
	cout << "All Objects File Load Success!" << endl;
	// ���� �ʿ� �����ϴ� ������ ������Ʈ�� ������ �ε��ϴ� �����Դϴ�.


}

int RoomManager::Create_room(int user_id) // ������� ��û������� ����մϴ�. 
{
	int return_create_room_number = -1;
	for (int i = 0; i < a_in_game_room.size(); ++i)
	{ 
		a_in_game_room[i]._room_state_lock.lock();
		if (a_in_game_room[i]._room_state == GAME_ROOM_STATE::FREE)
		{
			a_in_game_room[i].Create_Room(user_id, i, GAME_ROOM_STATE::READY);
			a_in_game_room[i]._room_state_lock.unlock();
			return_create_room_number = i;
			cout << return_create_room_number << "���� ����! " << endl;
			return return_create_room_number;
		}
		a_in_game_room[i]._room_state_lock.unlock();
	}

	return return_create_room_number;
}

void RoomManager::Clean_room(int room_number) // �濡 �ƹ��� ������� ���� ������ ������ �����Ƿ� ������� ��ȯ�մϴ�.
{
	a_in_game_room[room_number]._room_state = GAME_ROOM_STATE::FREE;
	a_in_game_room[room_number].Reset_Room();
}

bool RoomManager::Join_room(int user_id, int select_room_number) // �� ������ �õ��մϴ�. Ǯ���� ��� false�� �����մϴ�.
{
	if (a_in_game_room[select_room_number].Join_Player(user_id))
	{
		return true;
	}
	return false;
}

bool RoomManager::collision_player_player(int user_id) // ���� ��뿹���Դϴ�.
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id) // ���� ��� �����Դϴ�.
{
	return true;
}

Room* RoomManager::Get_Room_Info(int room_number) // �� ���� ��ü�� �ϰ������� �����մϴ�.
{
	return &a_in_game_room[room_number];
}