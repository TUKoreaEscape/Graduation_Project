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
		cs_packet_chat packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_CHAT;
		std::cout << "입력 : ";
		std::cin >> packet.message;
		packet.room_number = m_join_room_number;

		send_packet(&packet);
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
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no) {
				std::cout << "Socket Error Exit" << std::endl;
				TerminateProcess(info.hProcess, 1);
				m_shutdown = true;
				exit(0);
			}
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
		Input::GetInstance()->m_errorState = 1;
		break;
	}

	case SC_PACKET::SC_PACKET_LOGINOK:
	{
		//std::cout << "recv login ok" << std::endl;
		sc_packet_login_ok* recv_packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		m_pPlayer->SetID(recv_packet->id);

		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeNextState();

		cs_packet_request_all_room_info packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;
		packet.request_page = 0;
		
		send_packet(&packet);
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ID_OK:
	{
		Input::GetInstance()->m_SuccessState = 1;
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ID_FAIL:
	{
		Input::GetInstance()->m_errorState = 2;
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_SUCCESS:
	{
		set_join_room_state(true);
		join_voice_talk();
		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeNextState();
		break;
	}

	case SC_PACKET::SC_PACKET_CREATE_ROOM_OK:
	{
		set_join_room_state(true);
		join_voice_talk();
		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeNextState();
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL:
	{
		cs_packet_request_all_room_info packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;
		packet.request_page = m_page_num;
		send_packet(&packet);
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
		m_pPlayer->SetVelocity(packet->data.velocity);
		break;
	}

	case SC_PACKET::SC_PACKET_PUT_OTHER_PLAYER:
	{
		sc_put_player_packet* packet = reinterpret_cast<sc_put_player_packet*>(ptr);

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == -1)
			{
				m_ppOther[i]->SetID(packet->data.id);
				Other_Player_Pos[i].id = packet->data.id;
				m_ppOther[i]->SetVelocity(packet->data.velocity);
				if (packet->is_ready) {
					m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					m_ppOther[i]->SetTrackAnimationSet(0, 9);
				}
				else {
					m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					m_ppOther[i]->SetTrackAnimationSet(0, 0);
				}
				break;
			}
		}
		break;
	}

	case SC_PACKET::SC_PACKET_SELECT_TAGGER:
	{
		sc_packet_select_tagger* packet = reinterpret_cast<sc_packet_select_tagger*>(ptr);
		m_tagger_id = packet->id;
		if (m_pPlayer->GetID() == packet->id) {
			m_pPlayer->SetPlayerType(TYPE_TAGGER);
			m_before_player_type = TYPE_TAGGER;
		}
		else {
			m_pPlayer->SetPlayerType(TYPE_PLAYER);
			m_before_player_type = TYPE_PLAYER;
		}

		for (int i = 0; i < 5; ++i)
		{
			if (m_ppOther[i]->GetID() == packet->id)
				m_ppOther[i]->SetPlayerType(TYPE_TAGGER);
			else
				m_ppOther[i]->SetPlayerType(TYPE_PLAYER);
		}
		GameState& game_state = *GameState::GetInstance();
		game_state.ChangeNextState();
		break;
	}

	case SC_PACKET::SC_PACKET_PLAYER_EXIT:
	{
		Process_Player_Exit(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_CHAT:
	{
		Process_Chat(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_READY:
	{
		Process_Ready(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_INIT_POSITION:
	{
		Process_Init_Position(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_GAME_START:
	{
		Process_Game_Start(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_GAME_END:
	{
		Process_Game_End(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_LIFE_CHIP_UPDATE:
	{
		Process_LifeChip_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_TAGGER_CORRECT_LIFE_CHIP:
	{
		Process_Tagger_Collect_LifeChip(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_CUSTOMIZING:
	{
		sc_packet_customizing_update* packet = reinterpret_cast<sc_packet_customizing_update*>(ptr);

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

	case SC_PACKET::SC_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_TAGGER:
	{
		Process_ElectronicSystem_Reset_By_Tagger(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_PLAYER:
	{
		Process_ElectronicSystem_Reset_By_Player(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_DOOR_UPDATE:
	{
		Process_ElectronicSystemDoor_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_SWITCH_UPDATE:
	{
		Process_ElectronicSystem_Switch_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_LEVER_WORKING:
	{
		Process_ElectonicSystem_Lever_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ELECTRONIC_SYSTEM_ACTIVATE_UPDATE:
	{
		Process_ElectronicSystem_Activate(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ESCAPESYSTEM_ACTIVATE_UPDATE:
	{
		Process_Active_EscapeSystem(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_REQUEST_ESCAPESYSTEM_WORKING:
	{
		Process_EscapeSystem_Update(ptr); // 탈출장치를 조작 한 경우
		break;
	}

	case SC_PACKET::SC_PACKET_DOOR_UPDATE:
	{
		Process_Door_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_HIDDEN_DOOR_UPDATE:
	{
		Process_Hidden_Door_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_PICK_ITEM_INIT:
	{
		Process_Pick_Item_Init(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ITEM_BOX_UPDATE:
	{
		Process_Pick_Item_Box_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_PICK_ITEM_UPDATE:
	{
		Process_Pick_Item_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ACTIVATE_ALTAR:
	{
		Process_Active_Altar(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ALTAR_LIFECHIP_UPDATE:
	{
		Process_Altar_LifeChip_Update(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_ROOM_INFO:
	{
		sc_packet_request_room_info* packet = reinterpret_cast<sc_packet_request_room_info*>(ptr);
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

	case SC_PACKET::SC_PACKET_TAGGER_SKILL:
	{
		Process_Activate_Tagger_Skill(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_USE_FIRST_TAGGER_SKILL:
	{
		Process_Use_First_Tagger_Skill(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_USE_SECOND_TAGGER_SKILL:
	{
		Process_Use_Second_Tagger_Skill(ptr);
		break;
	}

	case SC_PACKET::SC_PACKET_USE_THIRD_TAGGER_SKILL:
	{
		Process_Use_Third_Tagger_Skill(ptr);
		break;
	}

	}
}

void Network::on_voice_talk()
{
	m_is_use_voice_talk = true;
	join_voice_talk();
	std::cout << "voice on" << std::endl;
}

void Network::off_voice_talk()
{
	exit_voice_talk();
	m_is_use_voice_talk = false;
	std::cout << "voice off" << std::endl;
}

void Network::join_voice_talk()
{
	if (m_is_use_voice_talk == false)
		return;

	if (false == get_voice_talk_working_state() && true == m_is_join_room) {
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

		set_voice_talk_working_state(true);
	}
}

void Network::exit_voice_talk()
{
	if (m_is_use_voice_talk == false)
		return;

	if (true == get_voice_talk_working_state()) {
		TerminateProcess(info.hProcess, 1);
		set_voice_talk_working_state(false);
	}
}