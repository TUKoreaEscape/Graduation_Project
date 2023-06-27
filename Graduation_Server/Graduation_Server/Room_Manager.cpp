#include "Room_Manager.h"

RoomManager::RoomManager()
{

}

RoomManager::~RoomManager()
{

}

void RoomManager::init() // 서버 시작 전 초기화해주는 함수입니다.
{
	// 서버 시작시 게임룸 초기화 장소
	for (int i = 0; i < a_in_game_room.size(); ++i) {
		a_in_game_room[i]._room_state = GAME_ROOM_STATE::FREE;
		a_in_game_room[i].init_room_number(i);
	}
}

void RoomManager::init_object() // 맵에 배치할 오브젝트를 로드해야하는곳입니다. (파일입출력 부분)
{
	// 여긴 맵에 존재하는 벽, 고정된 오브젝트의 충돌 정보를 서버에 로드하는 공간입니다.
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
		cout << "Walls BoundingBox : " << static_cast<int>((float)i / (float)nObjects * 100) << "%로드 완료\r";
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
		cout << "FixedObjects BoundingBox : " << static_cast<int>((float)i / (float)nObjects * 100) << "%로드 완료\r";
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
		// 여기서 door 추가할거임
		cout << "Doors BoundingBox : " << static_cast<int>((float)i / (float)6 * 100) << "%로드 완료\r";
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
	cout << "Doors BoundingBox : " << "100" << "%로드 완료\r";
	cout << "Door Load Success!                                       " << endl;

//======================= Electronic System Object Read =======================
	nReads = 0;
	nObjects = 0;
	nStrLength = 0;
	nReads = (unsigned int)fread(&nObjects, sizeof(int), 1, pFile);
	for (int i = 0; i < NUMBER_OF_ELECTRONIC; ++i)
	{
		// 여기서 door 추가할거임
		cout << "Electronic System : " << static_cast<int>((float)i / (float)6 * 100) << "%로드 완료\r";
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
	cout << "Electronic System : " << "100" << "%로드 완료\r";
	cout << "Electronic System Load Success!                            " << endl;

	cout << "All Objects File Load Success!" << endl;
	// 여긴 맵에 존재하는 고정된 오브젝트를 서버에 로드하는 공간입니다.


}

int RoomManager::Create_room(int user_id, int room_number) // 방생성을 요청받을경우 사용합니다. 
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

void RoomManager::Clean_room(int room_number) // 방에 아무도 없을경우 방이 존재할 이유가 없으므로 빈방으로 반환합니다.
{
	a_in_game_room[room_number]._room_state = GAME_ROOM_STATE::FREE;
	a_in_game_room[room_number].Reset_Room();
}

bool RoomManager::Join_room(int user_id, int select_room_number) // 방 접속을 시도합니다. 풀방인 경우 false를 리턴합니다.
{
	if (a_in_game_room[select_room_number].Join_Player(user_id))
	{
		return true;
	}
	return false;
}

bool RoomManager::collision_player_player(int user_id) // 추후 사용예정입니다.
{
	return true;
}

bool RoomManager::collision_wall_player(int user_id) // 추후 사용 예정입니다.
{
	return true;
}

Room* RoomManager::Get_Room_Info(int room_number) // 방 정보 전체를 일괄적으로 리턴합니다.
{
	return &a_in_game_room[room_number];
}