#pragma once
#include "Network.h"

Network* Network::NetworkInstance = nullptr;

Network::Network()
{

}

Network::~Network()
{

}

void Network::init_network()
{
	WSADATA WSAdata;

	if (WSAStartup(MAKEWORD(2, 2), &WSAdata) != 0) {

	}
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	int option = TRUE;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	int status = connect(m_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	std::cout << status << "<<<" << std::endl;
}

void Network::AssemblyPacket(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t make_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (io_byte != 0) {
		if (make_packet_size == 0) {
			make_packet_size = ptr[0];
			if (make_packet_size == 127)
			{
				make_packet_size *= ptr[2];
				make_packet_size += ptr[3];
			}
		}
		if (io_byte + saved_packet_size >= make_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, make_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ZeroMemory(packet_buffer, BUF_SIZE);
			ptr += make_packet_size - saved_packet_size;
			io_byte -= make_packet_size - saved_packet_size;
			//cout << "io byte - " << io_byte << endl;
			make_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void Network::listen_thread()
{
	while (true)
	{
		char buf[BUF_SIZE] = { 0 };
		WSABUF wsabuf{ BUF_SIZE, buf };
		DWORD recv_byte{ 0 }, recv_flag{ 0 };

		if (WSARecv(m_socket, &wsabuf, 1, &recv_byte, &recv_flag, nullptr, nullptr) == SOCKET_ERROR)
			;

		if (recv_byte > 0) {
			AssemblyPacket(wsabuf.buf, recv_byte);
		}
	}
}

void Network::ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	case SC_PACKET::NONE:

		break;

	case SC_PACKET::SC_PACKET_LOGINOK:
	{
		std::cout << "recv login ok" << std::endl;
		cs_packet_join_room packet;
		packet.size = sizeof(cs_packet_join_room);
		packet.type = CS_PACKET::CS_PACKET_JOIN_ROOM;
		packet.room_number = 0;
		send_packet(&packet);
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_SUCCESS:
	{
		std::cout << "방 접속 성공" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL:
	{
		std::cout << "recv SC_PACKET_JOIN_ROOM_FAIL" << std::endl;
		cs_packet_create_room packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_CREATE_ROOM;
		send_packet(&packet);
		break;
	}

	case SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE:
	{
		sc_other_player_move* packet = reinterpret_cast<sc_other_player_move*>(ptr);
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 5; ++j)
			{
				if (packet->data[i].id == m_ppOther[j]->GetID())
				{
					XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->data[i].position.x) / 100.f, static_cast<float>(packet->data[i].position.y) / 100.f, static_cast<float>(packet->data[i].position.z) / 100.f);
					XMFLOAT3 conversion_look = XMFLOAT3(static_cast<float>(packet->data[i].look.x) / 100.f, static_cast<float>(packet->data[i].look.y) / 100.f, static_cast<float>(packet->data[i].look.z) / 100.f);
					XMFLOAT3 conversion_right = XMFLOAT3(static_cast<float>(packet->data[i].right.x) / 100.f, static_cast<float>(packet->data[i].right.y) / 100.f, static_cast<float>(packet->data[i].right.z) / 100.f);
					//m_ppOther[j]->SetPosition(conversion_position, true);
					m_ppOther_Pos[j] = conversion_position;
					m_ppOther[j]->m_xmf3Look = conversion_look;
					m_ppOther[j]->m_xmf3Right = conversion_right;
					if (packet->data[i].input_key == DIR_FORWARD)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 1);
					}
					if (packet->data[i].input_key == DIR_BACKWARD)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 2);
					}
					if (packet->data[i].input_key == DIR_LEFT)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 3);
					}
					if (packet->data[i].input_key == DIR_RIGHT)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 4);
					}
					if (packet->data[i].input_key == DIR_UP)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 5);
					}

					if (packet->data[i].input_key == DIR_EMPTY)
					{
						m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[j]->SetTrackAnimationSet(0, 0);
					}
				}
			}
		}
		break;
	}

	case SC_PACKET::SC_PACKET_OTHER_PLAYER_DISCONNECT:
	{
		sc_other_player_disconnect* packet = reinterpret_cast<sc_other_player_disconnect*>(ptr);
		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == packet->id) {
				m_ppOther[i]->SetPosition(XMFLOAT3(-100, 100, -100));
				m_ppOther[i]->SetID(-1);
			}
		}
		break;
	}

	case SC_PACKET::SC_PACKET_CALCULATE_MOVE:
	{
		sc_packet_calculate_move* packet = reinterpret_cast<sc_packet_calculate_move*>(ptr);
		//m_pPlayer->SetPosition(packet->pos, true);
		XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->pos.x) / 100.f, static_cast<float>(packet->pos.y) / 100.f, static_cast<float>(packet->pos.z) / 100.f);
		pos_lock.lock();
		m_pPlayer_Pos = conversion_position;
		pos_lock.unlock();
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_PLAYER:
	{
		sc_put_player_packet* packet = reinterpret_cast<sc_put_player_packet*>(ptr);
		m_pPlayer->SetPosition(packet->data.position, true);
		m_pPlayer->SetID(packet->data.id);
		m_pPlayer->SetVelocity(packet->data.velocity);
		std::cout << "put player packet recv!" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER:
	{
		sc_put_player_packet* packet = reinterpret_cast<sc_put_player_packet*>(ptr);

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == -1)
			{
				std::cout << i << "번째에 플레이어 할당" << std::endl;
				m_ppOther[i]->SetID(packet->data.id);
				m_ppOther[i]->SetPosition(packet->data.position, true);
				m_ppOther[i]->SetVelocity(packet->data.velocity);
				break;
			}
		}
		std::cout << "put player_other packet recv!" << std::endl;

		for (int i = 0; i < 5; ++i)
		{
			std::cout << m_ppOther[i]->GetID() << std::endl;
		}
		
		break;
	}

	default:

		break;
	}
}

void Network::send_packet(void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];
	EXP_OVER* over = new EXP_OVER;
	over->m_comp_op = OP_SEND;
	
	ZeroMemory(&over->m_wsa_over, sizeof(over->m_wsa_over));
	over->m_wsa_buf.buf = reinterpret_cast<char*>(over->m_buf);
	over->m_wsa_buf.len = psize;
	memcpy(over->m_buf, packet, psize);
	int ret = WSASend(m_socket, &over->m_wsa_buf, 1, 0, 0, &over->m_wsa_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			;
	}
}