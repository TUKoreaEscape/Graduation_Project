#pragma once
#include "stdafx.h"
#include "Player.h"
#include "EXP_OVER.h"
#include "protocol.h"
#include "Scene.h"

#define  DIR_NO 100

class Network {
private:
	static Network* NetworkInstance;
	SOCKET			m_socket;
	const char*		SERVER_ADDR = "172.30.1.50";
public:
	std::mutex pos_lock;
	XMFLOAT3 m_pPlayer_Pos = { 0.0f, 0.0f, 0.0f };
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
	void AssemblyPacket(char* netbuf, size_t io_byte);
	void ProcessPacket(char* ptr);

	void send_packet(void* packet);
};