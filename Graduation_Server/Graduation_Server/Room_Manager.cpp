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
//======================= Fixed and Wall Object Read =======================
	FILE* pFile = nullptr;
	fopen_s(&pFile, "walls/FixedObjectsBounding0625.bin", "rb");
	if (pFile)
		rewind(pFile);

	unsigned int nReads = 0;
	unsigned int nObjects = 0;
	unsigned char nStrLength = 0;
	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };
	nReads = (unsigned int)fread(&nObjects, sizeof(int), 1, pFile);
	system("cls");
	for (unsigned int i = 0; i < nObjects; ++i)
	{
		cout << "Walls BoundingBox : " << static_cast<int>((float)i / (float)nObjects * 100) << "%�ε� �Ϸ�\r";
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrToken, sizeof(char), nStrLength, pFile); // <Wall>:
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrGameObjectName, sizeof(char), nStrLength, pFile);

		pstrGameObjectName[nStrLength] = '\0';

		float AABBCenter[3]{};
		float AABBExtents[3]{};
		float AABBQuats[4]{};

		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(pstrToken, sizeof(char), nStrLength, pFile); //"<BoundingBox>:"
		nReads = (unsigned int)fread(AABBCenter, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBExtents, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBQuats, sizeof(float), 4, pFile);
		//cout << "Object - " << i + 1 << " - " << pstrGameObjectName << " Center - (" << AABBCenter[0] << ", " << AABBCenter[1] << ", " << AABBCenter[2] << "), Extents - (" << AABBExtents[0] << ", " << AABBExtents[1] << ", " << AABBExtents[2] << ")" << endl;
		for (auto& _room : a_in_game_room) {
			_room.add_game_walls(Object_Type::OB_WALL, XMFLOAT3(AABBCenter[0], AABBCenter[1], AABBCenter[2]), XMFLOAT3(AABBExtents[0], AABBExtents[1], AABBExtents[2]));
		}
	}
	cout << "Walls and Fixed Object File Load Success!" << endl;

	nReads = (unsigned int)fread(&nObjects, sizeof(int), 1, pFile);
	for (unsigned int i = 0; i < nObjects; ++i)
	{
		cout << "FixedObjects BoundingBox : " << static_cast<int>((float)i / (float)nObjects * 100) << "%�ε� �Ϸ�\r";
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrToken, sizeof(char), nStrLength, pFile); // <Wall>:
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrGameObjectName, sizeof(char), nStrLength, pFile);

		pstrGameObjectName[nStrLength] = '\0';

		float AABBCenter[3]{};
		float AABBExtents[3]{};
		float AABBQuats[4]{};

		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(pstrToken, sizeof(char), nStrLength, pFile); //"<BoundingBox>:"
		nReads = (unsigned int)fread(AABBCenter, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBExtents, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBQuats, sizeof(float), 4, pFile);
		//cout << "Object - " << i + 1 << " - " << pstrGameObjectName << " Center - (" << AABBCenter[0] << ", " << AABBCenter[1] << ", " << AABBCenter[2] << "), Extents - (" << AABBExtents[0] << ", " << AABBExtents[1] << ", " << AABBExtents[2] << ")" << endl;
		for (auto& _room : a_in_game_room) {
			_room.add_fix_objects(Object_Type::OB_FIX, XMFLOAT3(AABBCenter[0], AABBCenter[1], AABBCenter[2]), XMFLOAT3(AABBExtents[0], AABBExtents[1], AABBExtents[2]));
		}
	}



//======================= Door Object Read =======================
	fopen_s(&pFile, "walls/DoorBB.bin", "rb");
	if (pFile)
		rewind(pFile);
	nReads = 0;
	nObjects = 0;
	nStrLength = 0;
	nReads = (unsigned int)fread(&nObjects, sizeof(int), 1, pFile);
	for (unsigned int i = 0; i < nObjects; ++i)
	{
		// ���⼭ door �߰��Ұ���
		cout << "Doors BoundingBox : " << static_cast<int>((float)i / (float)6 * 100) << "%�ε� �Ϸ�\r";
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrToken, sizeof(char), nStrLength, pFile); // <Wall>:
		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(&pstrGameObjectName, sizeof(char), nStrLength, pFile);

		pstrGameObjectName[nStrLength] = '\0';

		float AABBCenter[3]{};
		float AABBExtents[3]{};
		float AABBQuats[4]{};

		nReads = (unsigned int)fread(&nStrLength, sizeof(unsigned char), 1, pFile);
		nReads = (unsigned int)fread(pstrToken, sizeof(char), nStrLength, pFile); //"<BoundingBox>:"
		nReads = (unsigned int)fread(AABBCenter, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBExtents, sizeof(float), 3, pFile);
		nReads = (unsigned int)fread(AABBQuats, sizeof(float), 4, pFile);


		XMFLOAT3 center_pos = XMFLOAT3(AABBCenter[0], AABBCenter[1], AABBCenter[2]);
		XMFLOAT3 Extents = XMFLOAT3(AABBExtents[0], AABBExtents[1], AABBExtents[2]);
		//cout << "Door - " << i + 1 << " - " << pstrGameObjectName << " Center - (" << AABBCenter[0] << ", " << AABBCenter[1] << ", " << AABBCenter[2] << "), Extents - (" << AABBExtents[0] << ", " << AABBExtents[1] << ", " << AABBExtents[2] << ")" << endl;
		for (auto& _room : a_in_game_room)
		{
			_room.add_game_doors(i, OB_DOOR, center_pos, Extents);
		}
	}

	for (auto& _room : a_in_game_room)
	{
		for (int i = 0; i < MAX_INGAME_ITEM; ++i)
			_room.m_fix_item.emplace_back();
	}
	cout << "Doors BoundingBox : " << "100" << "%�ε� �Ϸ�\r";
	cout << "Door Load Success!                                       " << endl;

//======================= Electronic System Object Read =======================
	nReads = 0;
	nObjects = 0;
	nStrLength = 0;
	nReads = (unsigned int)fread(&nObjects, sizeof(int), 1, pFile);
	for (int i = 0; i < NUMBER_OF_ELECTRONIC; ++i)
	{
		// ���⼭ door �߰��Ұ���
		cout << "Electronic System : " << static_cast<int>((float)i / (float)6 * 100) << "%�ε� �Ϸ�\r";
		float AABBCenter[3]{ 0 };
		float AABBExtents[3]{ 0 };
		float AABBQuats[4]{ 0 };


		XMFLOAT3 center_pos = XMFLOAT3(AABBCenter[0], AABBCenter[1], AABBCenter[2]);
		XMFLOAT3 Extents = XMFLOAT3(AABBExtents[0], AABBExtents[1], AABBExtents[2]);
		for (auto& _room : a_in_game_room)
		{
			_room.add_game_ElectronicSystem(i, OB_ELECTRONICSYSTEM, center_pos, Extents);
		}
	}
	cout << "Electronic System : " << "100" << "%�ε� �Ϸ�\r";
	cout << "Electronic System Load Success!                            " << endl;

	cout << "All Objects File Load Success!" << endl;
	// ���� �ʿ� �����ϴ� ������ ������Ʈ�� ������ �ε��ϴ� �����Դϴ�.


}

int RoomManager::Create_room(int user_id, int room_number) // ������� ��û������� ����մϴ�. 
{
	int return_create_room_number = -1;


	if (a_in_game_room[room_number]._room_state == GAME_ROOM_STATE::FREE)
	{
		a_in_game_room[room_number]._room_state_lock.lock();
		a_in_game_room[room_number].Create_Room(user_id, room_number, GAME_ROOM_STATE::READY);
		a_in_game_room[room_number]._room_state_lock.unlock();
		return_create_room_number = room_number;
		return return_create_room_number;
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