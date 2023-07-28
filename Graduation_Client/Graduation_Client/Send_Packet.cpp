#pragma once
#include "Network.h"
#include "Game_state.h"
#include "Input.h"

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
		if (WSA_IO_PENDING != err_no) {
			disconnect_client();
			exit(0);
		}
	}
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

void Network::Send_Request_Room_Info(int page)
{
	cs_packet_request_all_room_info packet;

	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;
	packet.request_page = page;

	send_packet(&packet);
}

void Network::Send_Exit_Room()
{
	set_join_room_state(false);
	exit_voice_talk();
	cs_packet_request_exit_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_EXIT_ROOM;
	packet.request_page = m_page_num;

	send_packet(&packet);

	for (int i = 0; i < 5; ++i) {
		m_other_player_ready[i] = false;
		m_ppOther[i]->SetID(-1);
		m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
		m_ppOther[i]->SetTrackAnimationSet(0, 9);
	}
}

void Network::Send_Ready_Packet(bool is_ready)
{
	cs_packet_ready packet;

	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_READY;
	packet.ready_type = is_ready;

	send_packet(&packet);
}

void Network::Send_Loading_Success_Packet()
{
	cs_packet_loading_success packet;

	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_GAME_LOADING_SUCCESS;

	send_packet(&packet);
}

void Network::Send_Picking_Fix_Object_Packet(int box_index, GAME_ITEM::ITEM item_type)
{
	cs_packet_pick_fix_item packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_PICK_ITEM;
	packet.index = box_index;
	packet.item_type = item_type;

	send_packet(&packet);
}

void Network::Send_Fix_Object_Box_Update(short box_num, bool value)
{
	cs_packet_item_box_update packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_ITEM_BOX_UPDATE;
	packet.index = box_num;
	packet.is_open = value;

	send_packet(&packet);
}



void Network::Send_Attack_Packet()
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_ATTACK;

	send_packet(&packet);
}

void Network::Send_Use_Tagger_Skill(int skill_type)
{
	// 여긴 만약 술래인 경우!
	cs_packet_use_tagger_skill packet;
	packet.size = sizeof(packet);
	switch (skill_type)
	{
	case 1:
		packet.type = CS_PACKET::CS_PACKET_USE_FIRST_TAGGER_SKILL;
		break;

	case 2:
		packet.type = CS_PACKET::CS_PACKET_USE_SECOND_TAGGER_SKILL;
		break;

	case 3:
		packet.type = CS_PACKET::CS_PACKET_USE_THIRD_TAGGER_SKILL;
		break;
	}

	send_packet(&packet);
}

void Network::Send_Select_Room(int select_room_number, int index)
{
	switch (Input::GetInstance()->m_Roominfo[index].state)
	{

	case GAME_ROOM_STATE::FREE:
	{
		cs_packet_create_room packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_CREATE_ROOM;
		packet.room_number = select_room_number;
		//std::cout << "방 생성 패킷 전송" << std::endl;
		send_packet(&packet);
		break;
	}

	case GAME_ROOM_STATE::READY:
	{
		cs_packet_join_room packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_JOIN_ROOM;
		packet.room_number = select_room_number;

		send_packet(&packet);
		break;
	}

	case GAME_ROOM_STATE::PLAYING:
	{
		break;
	}

	}
}

void Network::Send_ElectronicSystem_Switch_Value(int system_index, int switch_index, bool value)
{
	cs_packet_request_eletronic_system_switch_control packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_SWICH;
	packet.electronic_system_index = system_index;
	packet.switch_idx = switch_index;
	packet.switch_value = value;

	send_packet(&packet);
}

void Network::Send_ElectronicSystem_Request_Activate(int system_index)
{
	cs_packet_request_electronic_system_activate packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_ATIVATE;
	packet.system_index = system_index;

	send_packet(&packet);
}

void Network::Send_Ativate_Altar()
{
	cs_packet_activate_altar packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_ACTIVATE_ALTAR;

	send_packet(&packet);
}

void Network::Send_Altar_Event()
{
	// 해당부분은 술래가 생명칩을 회수하였을 경우 서버로 전송합니다.
	cs_packet_altar_lifechip_update packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_ALTAR_LIFECHIP_UPDATE;

	send_packet(&packet);
}