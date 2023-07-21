#pragma once
#include "stdafx.h"
#include "Player.h"
#include "EXP_OVER.h"
#include "protocol.h"
#include "Scene.h"
#include "Server_Timer.h"

#define  DIR_NO 100
#define  USE_NETWORK 0
#define	 USE_VOICE 0

class Door;
class ItemBox;
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
	static Network*		NetworkInstance;
	Network();

private:
	SOCKET				m_socket;
	const char*			SERVER_ADDR = "172.30.1.52";
	Server_Timer		m_server_counter;
	Custom				data;

	SHELLEXECUTEINFO	info;
	HWND				hwnd_ExtMixerWin;
	bool				m_shutdown = false;
	int					m_my_id = -1;
	int					m_before_animation_index[5]{ 0 };

private: // 보이스톡이 사용하는 변수
	bool				m_is_use_voice_talk = false;
	bool				m_is_join_room = false;
	bool				m_voice_talk_in_working = false;


public: // 좌표를 동기화 시켜주는 변수들
	XMFLOAT3			m_pPlayer_Pos{ 0,0,0 };
	XMFLOAT3			m_pPlayer_before_Pos{ 0,0,0 };
	OtherPlayerPos		Other_Player_Pos[5]{};
	std::mutex			pos_lock;
	std::mutex			other_pos_lock;
	std::mutex			state_lock;
	bool				m_recv_move = false;

public: // 술래가 플레이시 사용하는 변수
	bool				m_lifechip = false;
	int					m_tagger_id = -1;
	bool				m_tagger_win = false;
	int					m_before_player_type = TYPE_PLAYER_YET;

public: // 엔딩창에 사용할 목적인 변수 (술래가 패배한 경우)
	int					m_escape_player_id[6];

public: // 방 선택창에서 사용하는 변수
	int					m_join_room_number = -1;
	int					m_page_num = 0;

public: // 스레드
	std::thread			send_thread;

public: // Ready 동기화를 위해 추가한 변수
	bool				m_other_player_ready[5]{ false };

public: // 클라이언트 오브젝트와 연결하는 용도
	Player*				m_pPlayer = nullptr;;
	Player**			m_ppOther = nullptr;
	GameObject**		m_UIPlay = nullptr;
	Door*				m_pDoors[6];
	InteractionObject*	m_pPowers[5];
	ItemBox*			m_pBoxes[MAX_INGAME_ITEM];
	GameObject*			m_Vents[NUM_VENT];
	InteractionObject*	m_Taggers_Box = nullptr;
	InteractionObject*	m_EscapeLevers[NUM_ESCAPE_LEVER];

public:
	static Network* GetInstance() {
		if (NetworkInstance == NULL) {
			NetworkInstance = new Network;
		}
		return NetworkInstance;
	}
	~Network();

	void init_network();


	void listen_thread();
	void Debug_send_thread();
	void AssemblyPacket(char* netbuf, size_t io_byte);
	void ProcessPacket(char* ptr);

	void Process_Player_Move(char* ptr);
	void Process_Other_Player_Move(char* ptr);
	void Process_Other_Move(char* ptr);
	void Process_Player_Exit(char* ptr);
	void Process_Ready(char* ptr);
	void Process_Init_Position(char* ptr);
	void Process_Chat(char* ptr);
	void Process_Game_Start(char* ptr);
	void Process_Game_End(char* ptr);
	void Process_Door_Update(char* ptr);
	void Process_Hidden_Door_Update(char* ptr);
	void Process_ElectronicSystem_Reset_By_Tagger(char* ptr);
	void Process_ElectronicSystem_Reset_By_Player(char* ptr);
	void Process_ElectronicSystemDoor_Update(char* ptr);
	void Process_ElectonicSystem_Lever_Update(char* ptr);
	void Process_ElectrinicSystem_Init(char* ptr);
	void Process_ElectronicSystem_Switch_Update(char* ptr);
	void Process_ElectronicSystem_Activate(char* ptr);
	void Process_EscapeSystem_Update(char* ptr);
	void Process_Attack_Packet(char* ptr);
	void Process_LifeChip_Update(char* ptr);
	void Process_Tagger_Collect_LifeChip(char* ptr);
	void Process_Pick_Item_Init(char* ptr);
	void Process_Pick_Item_Box_Update(char* ptr);
	void Process_Pick_Item_Update(char* ptr);
	void Process_Active_Altar(char* ptr);
	void Process_Active_EscapeSystem(char* ptr);
	void Process_Altar_LifeChip_Update(char* ptr);

	// 술래 스킬 적용하는 부분!
	void Process_Activate_Tagger_Skill(char* ptr);
	void Process_Use_First_Tagger_Skill(char* ptr);
	void Process_Use_Second_Tagger_Skill(char* ptr);
	void Process_Use_Third_Tagger_Skill(char* ptr);

	// 게임 시작 전 사용하는 함수들
	void Send_Request_Room_Info(int page);
	void Send_Exit_Room();
	void Send_Customizing_Data();
	void Send_Ready_Packet(bool is_ready);
	void Send_Loading_Success_Packet();
	void Send_Select_Room(int select_room_number, int index);
	void Send_ElectronicSystem_Switch_Value(int system_index, int switch_index, bool value);
	void Send_ElectronicSystem_Request_Activate(int system_index);
	void Send_Ativate_Altar();
	void Send_Altar_Event();

	// 게임 플레이시 사용하는 패킷전송 함수
	void Send_Use_Tagger_Skill(int skill_type);
	void Send_Picking_Fix_Object_Packet(int box_index, GAME_ITEM::ITEM item_type);
	void Send_Fix_Object_Box_Update(short box_num, bool value);
	void Send_Attack_Packet();

	void send_packet(void* packet);

public:
	std::wstring StringToWString(const std::string& str)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* buffer = new wchar_t[length];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, length);
		std::wstring result(buffer);
		delete[] buffer;
		return result;
	}

public: // 보이스톡 관련 함수
	void on_voice_talk();
	void off_voice_talk();

	void set_voice_talk_working_state(bool value) { m_voice_talk_in_working = value; }
	bool get_voice_talk_working_state() { return m_voice_talk_in_working; }

	void join_voice_talk();
	void exit_voice_talk();

	void set_join_room_state(bool value) { m_is_join_room = value; }
	bool get_join_room_state() { return m_is_join_room; }
};