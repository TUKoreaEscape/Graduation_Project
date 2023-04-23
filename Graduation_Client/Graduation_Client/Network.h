#pragma once
#include "stdafx.h"
#include "Player.h"
#include "EXP_OVER.h"
#include "protocol.h"
#include "Scene.h"
#include "Server_Timer.h"

#define  DIR_NO 100

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
	XMFLOAT3 Other_Pos;
	std::mutex pos_lock;
};

class Network {
private:
	static Network* NetworkInstance;
	SOCKET			m_socket;
	const char*		SERVER_ADDR = "183.101.112.30";
	Server_Timer	m_server_counter;
	Custom			data;
public:
	std::mutex pos_lock;
	bool	m_recv_move = false;
	int		m_my_id = -1;

	XMFLOAT3 m_pPlayer_Pos{ 0,0,0 };
	OtherPlayerPos Other_Player_Pos[5]{};
	Player* m_pPlayer = nullptr;;
	Player** m_ppOther = nullptr;

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
	void Send_Customizing_Data();
	void AssemblyPacket(char* netbuf, size_t io_byte);
	void ProcessPacket(char* ptr);

	void send_packet(void* packet);
};