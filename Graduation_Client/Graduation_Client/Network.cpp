#pragma once
#include "Network.h"

Network* Network::NetworkInstance = nullptr;

Network::Network()
{

}

Network::~Network()
{
	TerminateProcess(info.hProcess, 1);
}

void Network::Send_Customizing_Data()
{
	cs_packet_customizing_update packet;

	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_CUSTOMIZING;

	packet.head = data.head;
	packet.body = data.body;
	packet.body_parts = data.body_parts;
	packet.eyes = data.eyes;
	packet.gloves = data.gloves;
	packet.mouthandnoses = data.mouthandnoses;

	send_packet(&packet);
}

void Network::Debug_send_thread()
{
	while (true)
	{
		int code;
		std::cout << "1. Head 변경 \n";
		std::cout << "2. Head Part 변경 (쓰지마셈)\n";
		std::cout << "3. Body 변경 \n";
		std::cout << "4. Body Part 변경 \n";
		std::cout << "5. Eye 변경 \n";
		std::cout << "6. Glove 변경 \n";
		std::cout << "7. Mouse and Nose 변경 \n";
		std::cout << "8. 무조건 한번 눌러요\n";
		std::cout << "명령어 입력 : ";
		std::cin >> code;

		int select;
		switch (code)
		{
		case 1:
		{
			system("cls");
			std::cout << "Head Select (0 ~ 20) : ";
			std::cin >> select;

			GameObject::SetParts(0, 5, select);
			data.head = static_cast<HEADS>(select);
			Send_Customizing_Data();
			system("cls");
			break;
		}

		case 3:
			system("cls");
			std::cout << "Body Select (0 ~ 5) : ";
			std::cin >> select;
			GameObject::SetParts(0, 0, select);
			data.body = static_cast<BODIES>(select);
			Send_Customizing_Data();
			system("cls");
			break;

		case 4:
			system("cls");
			std::cout << "Body Parts Select (0 ~ 5) : ";
			std::cin >> select;
			GameObject::SetParts(0, 1, select);
			data.body_parts = static_cast<BODYPARTS>(select);
			Send_Customizing_Data();
			system("cls");
			break;

		case 5:
			system("cls");
			std::cout << "Eyes Select (0 ~ 6) : ";
			std::cin >> select;

			GameObject::SetParts(0, 2, select);
			data.eyes = static_cast<EYES>(select);
			Send_Customizing_Data();
			system("cls");
			break;

		case 6:
			system("cls");
			std::cout << "Gloves Select (0 ~ 5) : ";
			std::cin >> select;
			
			data.gloves = static_cast<GLOVES>(select);
			GameObject::SetParts(0, 3, select);
			Send_Customizing_Data();
			system("cls");
			break;

		case 7:
			system("cls");
			std::cout << "Mouse And Nose Select (0 ~ 6) : ";
			std::cin >> select;
			GameObject::SetParts(0, 4, select);
			data.mouthandnoses = static_cast<MOUTHANDNOSES>(select);
			Send_Customizing_Data();
			system("cls");
			break;

		case 8:
			GameObject::SetParts(0, 0, 0);
			data.head = static_cast<HEADS>(0);
			data.body_parts = static_cast<BODYPARTS>(0);
			data.eyes = static_cast<EYES>(0);
			data.gloves = static_cast<GLOVES>(0);
			data.body = static_cast<BODIES>(0);
			data.mouthandnoses = static_cast<MOUTHANDNOSES>(0);
			GameObject::SetParts(0, 1, 0);
			GameObject::SetParts(0, 2, 0);
			GameObject::SetParts(0, 3, 0);
			GameObject::SetParts(0, 4, 0);
			GameObject::SetParts(0, 5, 0);

			Send_Customizing_Data();
			break;

		case 9:
		{
			TerminateProcess(info.hProcess, 1);
			break;
		}
		}
	}
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
		
	case SC_PACKET::SC_PACKET_LOGINFAIL:
	{
		std::cout << "========================================" << std::endl;
		std::cout << ">>>>>> 로그인에 실패하였습니다@ <<<<<<" << std::endl;
		std::cout << "========================================" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_LOGINOK:
	{
		//std::cout << "recv login ok" << std::endl;
		sc_packet_login_ok* recv_packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		m_pPlayer->SetID(recv_packet->id);

		m_login = true;
		std::cout << "========================================" << std::endl;
		std::cout << ">>>>>> 로그인에 성공하였습니다! <<<<<<" << std::endl;
		std::cout << "========================================" << std::endl;
		cs_packet_join_room packet;
		packet.size = sizeof(cs_packet_join_room);
		packet.type = CS_PACKET::CS_PACKET_JOIN_ROOM;
		packet.room_number = 0;
		send_packet(&packet);
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ID_OK:
	{
		std::cout << "========================================" << std::endl;
		std::cout << ">>> 아이디 생성에 성공하였습니다! <<<" << std::endl;
		std::cout << "========================================" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ID_FAIL:
	{
		std::cout << "========================================" << std::endl;
		std::cout << ">>> 아이디 생성에 실패하였습니다! <<<" << std::endl;
		std::cout << "========================================" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_SUCCESS:
	{
		//std::cout << "방 접속 성공" << std::endl;
		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.hwnd = NULL;
		info.lpVerb = L"open";
		info.lpFile = L"voice\\Voice.exe";
		info.lpParameters = NULL;
		info.lpDirectory = NULL;
		info.nShow = SW_HIDE;
		info.hInstApp = NULL;

		ShellExecuteEx(&info); // start process

		GetProcessId(info.hProcess); // retrieve PID
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ROOM_OK:
	{
		//std::cout << "보이스 시작" << std::endl;
		//ShellExecute(NULL, L"open", L"voice\Voice.exe", NULL, NULL, SW_SHOWMINIMIZED);

		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.hwnd = NULL;
		info.lpVerb = L"open";
		info.lpFile = L"voice\\Voice.exe";
		info.lpParameters = NULL;
		info.lpDirectory = NULL;
		info.nShow = SW_HIDE;
		info.hInstApp = NULL;

		ShellExecuteEx(&info); // start process

		GetProcessId(info.hProcess); // retrieve PID
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL:
	{
		//std::cout << "recv SC_PACKET_JOIN_ROOM_FAIL" << std::endl;
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
					//m_ppOther[j]->SetPosition(conversion_position);
					Other_Player_Pos[j].pos_lock.lock();
					Other_Player_Pos[j].Other_Pos = conversion_position;
					Other_Player_Pos[j].pos_lock.unlock();
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
				Other_Player_Pos[i].id = -1;
				Other_Player_Pos[i].Other_Pos = XMFLOAT3(-100, 100, -100);
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
		//std::cout << "put player packet recv!" << std::endl;
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER:
	{
		sc_put_player_packet* packet = reinterpret_cast<sc_put_player_packet*>(ptr);

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == -1)
			{
				//std::cout << i << "번째에 플레이어 할당" << std::endl;
				m_ppOther[i]->SetID(packet->data.id);
				Other_Player_Pos[i].id = packet->data.id;
				m_ppOther[i]->SetPosition(packet->data.position, true);
				m_ppOther[i]->SetVelocity(packet->data.velocity);
				break;
			}
		}
		//std::cout << "put player_other packet recv!" << std::endl;	
		break;
	}

	case SC_PACKET::SC_PACKET_CUSTOMIZING:
	{
		sc_packet_customizing_update* packet = reinterpret_cast<sc_packet_customizing_update*>(ptr);
		//packet->body;
		if (packet->id == m_pPlayer->GetID())
		{
			GameObject::SetParts(0, 0, packet->body);
			GameObject::SetParts(0, 1, packet->body_parts);
			GameObject::SetParts(0, 2, packet->eyes);
			GameObject::SetParts(0, 3, packet->gloves);
			GameObject::SetParts(0, 4, packet->mouthandnoses);
			GameObject::SetParts(0, 5, packet->head);

			data.body = static_cast<BODIES>(packet->body);
			data.body_parts = static_cast<BODYPARTS>(packet->body_parts);
			data.eyes = static_cast<EYES>(packet->eyes);
			data.gloves = static_cast<GLOVES>(packet->gloves);
			data.head = static_cast<HEADS>(packet->head);
			data.mouthandnoses = static_cast<MOUTHANDNOSES>(packet->mouthandnoses);
		}

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == packet->id)
			{
				GameObject::SetParts(i + 1, 0, static_cast<int>(packet->body));
				GameObject::SetParts(i + 1, 1, static_cast<int>(packet->body_parts));
				GameObject::SetParts(i + 1, 2, static_cast<int>(packet->eyes));
				GameObject::SetParts(i + 1, 3, static_cast<int>(packet->gloves));
				GameObject::SetParts(i + 1, 4, static_cast<int>(packet->mouthandnoses));
				GameObject::SetParts(i + 1, 5, static_cast<int>(packet->head));
			}
		}
		break;
	}
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
			exit(0);
	}
}