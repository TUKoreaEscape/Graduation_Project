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

	case SC_PACKET::SC_LOGINOK:
	{
		std::cout << "recv login ok" << std::endl;
		cs_packet_join_room packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_JOIN_ROOM;
		packet.room_number = 0;
		send_packet(&packet);
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
	case SC_PACKET::SC_PACKET_MOVE:
	{
		sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);
		
		if (packet->id != m_pPlayer->GetID()) 
		{
			for (int i = 0; i < 5; ++i)
			{
				if (m_ppOther[i]->GetID() == packet->id)
				{
					m_ppOther[i]->SetPosition(packet->pos, true);
					m_ppOther[i]->m_xmf3Look = XMFLOAT3(static_cast<float>(packet->look[0]) / 100, static_cast<float>(packet->look[1]) / 100, static_cast<float>(packet->look[2]) / 100);
					m_ppOther[i]->m_xmf3Right = XMFLOAT3(static_cast<float>(packet->right[0]) / 100, static_cast<float>(packet->right[1]) / 100, static_cast<float>(packet->right[2]) / 100);
					if (packet->input_key == DIR_FORWARD)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 1);
					}
					if (packet->input_key == DIR_BACKWARD)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 2);
					}
					if (packet->input_key == DIR_LEFT)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 3);
					}
					if (packet->input_key == DIR_RIGHT)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 4);
					}
					if (packet->input_key == DIR_UP)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 5);
					}

					if (packet->input_key == DIR_EMPTY)
					{
						m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
						m_ppOther[i]->SetTrackAnimationSet(0, 0);
					}
					break;
				}
			}
		}

		else
		{
			m_pPlayer->m_position_lock.lock();
			m_pPlayer->SetPosition(packet->pos, true);
			m_pPlayer->m_position_lock.unlock();
			// 회전각도 여기 추가해야할수도?
		}

		break;
	}

	case SC_PACKET::SC_USER_ROTATE:
	{
		sc_packet_player_rotate* packet = reinterpret_cast<sc_packet_player_rotate*>(ptr);
		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == packet->id)
			{
				m_ppOther[i]->m_xmf3Look = packet->xmf3Look;
				m_ppOther[i]->m_xmf3Right = packet->xmf3Right;
				m_ppOther[i]->m_xmf3Up = packet->m_xmf3Up;
				break;
			}
		}

		break;
	}

	case SC_PACKET::SC_USER_UPDATE:
	{
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_PLAYER:
	{
		sc_update_user_packet* packet = reinterpret_cast<sc_update_user_packet*>(ptr);
		m_pPlayer->SetPosition(packet->data.position);
		m_pPlayer->SetID(packet->data.id);
		m_pPlayer->SetVelocity(packet->data.velocity);
		std::cout << "put player packet recv!" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER:
	{
		sc_update_user_packet* packet = reinterpret_cast<sc_update_user_packet*>(ptr);

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
	memcpy(over->m_buf, packet, psize);
	ZeroMemory(&over->m_wsa_over, sizeof(over->m_wsa_over));
	over->m_wsa_buf.buf = reinterpret_cast<CHAR*>(over->m_buf);
	over->m_wsa_buf.len = psize;
	int ret = WSASend(m_socket, &over->m_wsa_buf, 1, NULL, 0, &over->m_wsa_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			;
	}
}