#pragma once
#include "Network.h"
#include "Game_state.h"
#include "Input.h"
Network* Network::NetworkInstance = nullptr;

Network::Network()
{

}

Network::~Network()
{
	m_shutdown = true;
	TerminateProcess(info.hProcess, 1);
}


void Network::Debug_send_thread()
{
	while (m_shutdown == false)
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
		std::cout << "9. 프로그램 전체 종료 \n";
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
			system("cls");
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
			system("cls");
			TerminateProcess(info.hProcess, 1);
			break;
		}

		case 10:
			system("cls");
			Send_Ready_Packet(true);
			break;
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

	unsigned char make_size;
	while (io_byte != 0) {
		if (make_packet_size == 0) {
			make_size = ptr[0];
			make_packet_size = make_size;
			//std::cout << "packet size : " << make_packet_size << std::endl;
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
	while (false == m_shutdown)
	{
		char buf[BUF_SIZE] = { 0 };
		WSABUF wsabuf{ BUF_SIZE, buf };
		DWORD recv_byte{ 0 }, recv_flag{ 0 };

		if (WSARecv(m_socket, &wsabuf, 1, &recv_byte, &recv_flag, nullptr, nullptr) == SOCKET_ERROR)
		{
			std::cout << "Socket Error Exit" << std::endl;
			TerminateProcess(info.hProcess, 1);
			exit(0);
		}

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

		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeState();

		cs_packet_request_all_room_info packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;
		packet.request_page = 0;
		
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
		m_join_room = true;
		send_thread = std::thread{ &Network::Debug_send_thread, this };
		for (int i = 0; i < 5; ++i)
			m_ppOther[i]->SetPosition(XMFLOAT3(-100, -100, -100));
#if USE_VOICE
		std::wstring parameter = L"addsession -l ";
		std::wstring room_parameter = std::to_wstring(m_join_room_number);
		std::wstring room_parameter2 = L"lobby";
		std::wstring option_parameter = L" -audio yes";

		std::wstring result_parameter = parameter + room_parameter + room_parameter2 + option_parameter;

		std::wcout << result_parameter << std::endl;
		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.hwnd = NULL;
		info.lpVerb = L"open";
		info.lpFile = L"voice\\Voice.exe";
		info.lpParameters = (LPCWSTR&)result_parameter;
		info.lpDirectory = NULL;
		info.nShow = SW_HIDE;
		info.hInstApp = NULL;

		ShellExecuteEx(&info); // start process

		GetProcessId(info.hProcess); // retrieve PID
#endif

		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeState();

		Send_Ready_Packet(true);
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ROOM_OK:
	{
		//std::cout << "보이스 시작" << std::endl;
		//ShellExecute(NULL, L"open", L"voice\Voice.exe", NULL, NULL, SW_SHOWMINIMIZED);
		//std::cout << "방 생성에 성공하였습니다." << std::endl;
		m_join_room = true;
		send_thread = std::thread{ &Network::Debug_send_thread, this };
		for (int i = 0; i < 5; ++i)
			m_ppOther[i]->SetPosition(XMFLOAT3(-100, -100, -100));
#if USE_VOICE
		std::wstring parameter = L"addsession -l ";
		std::wstring room_parameter = std::to_wstring(m_join_room_number);
		std::wstring room_parameter2 = L"lobby";
		std::wstring option_parameter = L" -audio yes";

		std::wstring result_parameter = parameter + room_parameter + room_parameter2 + option_parameter;

		std::wcout << result_parameter << std::endl;
		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.hwnd = NULL;
		info.lpVerb = L"open";
		info.lpFile = L"voice\\Voice.exe";
		info.lpParameters = (LPCWSTR&)result_parameter;
		info.lpDirectory = NULL;
		info.nShow = SW_HIDE;
		info.hInstApp = NULL;

		ShellExecuteEx(&info); // start process

		GetProcessId(info.hProcess); // retrieve PID
#endif
		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeState();

		Send_Ready_Packet(true);
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL:
	{
		//std::cout << "recv SC_PACKET_JOIN_ROOM_FAIL" << std::endl;
		
		std::cout << "방 접속에 실패하였습니다." << std::endl;
		cs_packet_request_all_room_info packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;
		packet.request_page = 0;

		system("cls");
		//send_packet(&packet);
		//cs_packet_create_room packet;
		//packet.size = sizeof(packet);
		//packet.type = CS_PACKET::CS_PACKET_CREATE_ROOM;
		//send_packet(&packet);
		break;
	}

	case SC_PACKET::SC_PACKET_OTHER_PLAYER_UPDATE:
	{
		Process_Other_Player_Move(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ATTACK:

		break;

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
		Process_Player_Move(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_MOVE:
	{
		Process_Other_Move(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_PLAYER:
	{
		sc_put_player_packet* packet = reinterpret_cast<sc_put_player_packet*>(ptr);
		m_pPlayer->SetID(packet->data.id);
		m_pPlayer->SetPosition(packet->data.position, true);
		//std::cout << "Set Init Pos : (" << packet->data.position.x << ", " << packet->data.position.y << ", " << packet->data.position.z << ") " << std::endl;
		m_pPlayer->SetVelocity(packet->data.velocity);
		m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
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
				m_ppOther[i]->SetPlayerType(TYPE_PLAYER_YET);
				break;
			}
		}
		//std::cout << "put player_other packet recv!" << std::endl;	
		break;
	}

	case SC_PACKET::SC_PACKET_SELECT_TAGGER:
	{
		sc_packet_select_tagger* packet = reinterpret_cast<sc_packet_select_tagger*>(ptr);
		
		if (m_pPlayer->GetID() == packet->id)
			m_pPlayer->SetPlayerType(TYPE_TAGGER);
		else
			m_pPlayer->SetPlayerType(TYPE_PLAYER);

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == packet->id)
				m_ppOther[i]->SetPlayerType(TYPE_TAGGER);
			else
				m_ppOther[i]->SetPlayerType(TYPE_PLAYER);
		}

		std::cout << std::endl;
		std::cout << "Player (Server ID) [" << packet->id << "] 가 술래로 결정되었습니다. 외곽선이 빨간선으로 바뀝니다." << std::endl;

		break;
	}

	case SC_PACKET::SC_PACKET_GAME_START:
	{
		Process_Game_Start(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_LIFE_CHIP_UPDATE:
	{
		// 게임 시작시 생명칩을 활성화 시켜줘야하므로 존재함.
		Process_LifeChip_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_CUSTOMIZING:
	{
		sc_packet_customizing_update* packet = reinterpret_cast<sc_packet_customizing_update*>(ptr);
		//packet->body;
		//std::cout << "=======================================================================" << std::endl;
		//std::cout << "packet->id : " << packet->id << std::endl;
		//std::cout << "body : " << static_cast<BODIES>(packet->body) << std::endl;
		//std::cout << "body_parts : " << static_cast<BODYPARTS>(packet->body_parts) << std::endl;
		//std::cout << "eyes : " << static_cast<EYES>(packet->eyes) << std::endl;
		//std::cout << "gloves : " << static_cast<GLOVES>(packet->gloves) << std::endl;
		//std::cout << "mouthandnoses : " << static_cast<MOUTHANDNOSES>(packet->mouthandnoses) << std::endl;
		//std::cout << "head : " << static_cast<HEADS>(packet->head) << std::endl;
		//std::cout << "=======================================================================" << std::endl;

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
			if (m_ppOther[i]->GetID() == packet->id && packet->id != -1)
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

	case SC_PACKET::SC_PACKET_ELECTRONIC_SWITCH_INIT:
	{
		Process_ElectrinicSystem_Init(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_DOOR_UPDATE:
	{
		Process_ElectronicSystemDoor_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_SWITCH_UPDATE:
	{
		// 아직 처리할 전력장치 코드가 없음
		break;
	}

	case SC_PACKET::SC_PACKET_DOOR_UPDATE:
	{
		Process_Door_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_PICK_ITEM_INIT:
	{
		Process_Pick_Item_Init(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_PICK_ITEM_UPDATE:
	{
		break;
	}

	case SC_PACKET::SC_PACKET_ACTIVATE_ALTAR:
	{
		break;
	}

	case SC_PACKET::SC_PACKET_ALTAR_LIFECHIP_UPDATE:
	{
		break;
	}

	case SC_PACKET::SC_PACKET_ROOM_INFO:
	{
		sc_packet_request_room_info* packet = reinterpret_cast<sc_packet_request_room_info*>(ptr);
		system("cls");
		for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i)
		{
			memcpy(Input::GetInstance()->m_Roominfo[i].room_name, packet->room_info[i].room_name, sizeof(packet->room_info[i].room_name));
			Input::GetInstance()->m_Roominfo[i].room_number = packet->room_info[i].room_number;
			Input::GetInstance()->m_Roominfo[i].join_member = packet->room_info[i].join_member;
			Input::GetInstance()->m_Roominfo[i].state = packet->room_info[i].state;
		}
		break;
	}

	case SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE:
	{
		sc_packet_update_room* packet = reinterpret_cast<sc_packet_update_room*>(ptr);
		int calcul_num = packet->room_number % 6;
		Input::GetInstance()->m_Roominfo[calcul_num].join_member = packet->join_member;
		Input::GetInstance()->m_Roominfo[calcul_num].state = packet->state;
		Input::GetInstance()->m_Roominfo[calcul_num].room_number = packet->room_number;
		break;
	}

	}
}

