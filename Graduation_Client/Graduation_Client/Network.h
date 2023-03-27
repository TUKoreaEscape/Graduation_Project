#pragma once
#include "stdafx.h"
#include "Player.h"
#include "EXP_OVER.h"
#include "protocol.h"
#include "Scene.h"

class Network {
private:
	static Network* NetworkInstance;
	SOCKET			m_socket;
	const char*		SERVER_ADDR = "127.0.0.1";
public:
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