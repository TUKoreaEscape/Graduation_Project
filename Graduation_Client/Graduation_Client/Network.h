#pragma once
#include "stdafx.h"
#include "Player.h"
#include "EXP_OVER.h"
#include "protocol.h"
#include "Scene.h"
#include "Server_Timer.h"

#define  DIR_NO 100
#define  USE_NETWORK 1
#define	 USE_VOICE 0

class Door;
class InteractionObject;

struct Custom {
	HEADS			head;
	BODIES			body;
	BODYPARTS		body_parts;
	EYES			eyes;
	GLOVES			gloves;
	MOUTHANDNOSES	mouthandnoses;
};

struct OtherPlayerPos {
	short	id;
	XMFLOAT3 Other_Pos{};
	std::mutex pos_lock;
};

class Network {
private:
	static Network* NetworkInstance;
	SOCKET			m_socket;
	const char*		SERVER_ADDR = "127.0.0.1";
	Server_Timer	m_server_counter;
	Custom			data;

	SHELLEXECUTEINFO	info;
	HWND				hwnd_ExtMixerWin;
	bool				m_shutdown = false;
	bool				m_lifechip = false;
public:
	std::mutex pos_lock;
	std::mutex other_pos_lock;
	bool	m_recv_move = false;
	int		m_my_id = -1;
	int		m_join_room_number = -1;

	std::thread send_thread;
	//임시사용 변수입니다.
	bool    m_login = false;
	bool	m_join_room = false;

	XMFLOAT3 m_pPlayer_Pos{ 0,0,0 };
	XMFLOAT3 m_pPlayer_before_Pos{ 0,0,0 };
	OtherPlayerPos Other_Player_Pos[5]{};
	Player* m_pPlayer = nullptr;;
	Player** m_ppOther = nullptr;
	Door* m_pDoors[6];
	InteractionObject* m_pPowers[5];

	static Network* GetInstance() {
		if (NetworkInstance == NULL) {
			NetworkInstance = new Network;
		}
		return NetworkInstance;
	}
	Network();
	~Network();

	void init_network();


	void listen_thread();
	void Debug_send_thread();
	void AssemblyPacket(char* netbuf, size_t io_byte);
	void ProcessPacket(char* ptr);

	void Process_Player_Move(char* ptr);
	void Process_Other_Player_Move(char* ptr);
	void Process_Other_Move(char* ptr);
	void Process_Game_Start(char* ptr);
	void Process_Door_Update(char* ptr);
	void Process_ElectronicSystemDoor_Update(char* ptr);
	void Process_Attack_Packet(char* ptr);
	void Process_ElectrinicSystem_Init(char* ptr);
	void Process_LifeChip_Update(char* ptr);
	void Process_Pick_Item_Init(char* ptr);
	void Process_Pick_Item_Update(char* ptr);
	void Process_Active_Altar(char* ptr);
	void Process_Altar_LifeChip_Update(char* ptr);

	// 게임 시작 전 사용하는 함수들
	void Send_Request_Room_Info(int page);
	void Send_Customizing_Data();
	void Send_Ready_Packet(bool is_ready);
	void Send_Loading_Success_Packet();
	void Send_Select_Room(int select_room_number);
	void Send_Ativate_Altar();
	void Send_Altar_Event();

	// 게임 플레이시 사용하는 패킷전송 함수
	void Send_Use_Tagger_Skill(int skill_type);
	void Send_Picking_Fix_Object_Packet(short item_type);
	void Send_Attack_Packet();

	void send_packet(void* packet);
};