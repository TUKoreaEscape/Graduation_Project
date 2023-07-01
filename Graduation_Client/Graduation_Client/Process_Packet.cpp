#include "Network.h"
#include "Object.h"
#include "GameObject.h"

void Network::Process_Game_Start(char* ptr)
{
	sc_packet_game_start* packet = reinterpret_cast<sc_packet_game_start*>(ptr);

	GameState& game_state = *GameState::GetInstance();
	game_state.ChangeNextState();
}

void Network::Process_Game_End(char* ptr)
{
	sc_packet_game_end* packet = reinterpret_cast<sc_packet_game_end*>(ptr);
	
	GameState& game_state = *GameState::GetInstance();
	game_state.ChangeNextState();

	m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
	for (int i = 0; i < 5; ++i)
		m_ppOther[i]->SetPlayerType(TYPE_PLAYER_YET);
}

void Network::Process_LifeChip_Update(char* ptr)
{
	sc_packet_life_chip_update* packet = reinterpret_cast<sc_packet_life_chip_update*>(ptr);
	packet->id;
	packet->life_chip;
}

void Network::Process_Player_Move(char* ptr)
{
	m_pPlayer_before_Pos = m_pPlayer_Pos;
	sc_packet_calculate_move* packet = reinterpret_cast<sc_packet_calculate_move*>(ptr);
	//m_pPlayer->SetPosition(packet->pos);	
	XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->pos.x) / 10000.f, static_cast<float>(packet->pos.y) / 10000.f, static_cast<float>(packet->pos.z) / 10000.f);
	pos_lock.lock();
	m_pPlayer_Pos = conversion_position;
	m_pPlayer->SetIsColledUpFace(packet->is_collision_up_face);
	pos_lock.unlock();
}

void Network::Process_Other_Move(char* ptr)
{
	sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);

	for (int i = 0; i < 5; ++i)
	{
		if (packet->data.id != m_ppOther[i]->GetID())
			continue;
		XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->data.position.x) / 10000.f, static_cast<float>(packet->data.position.y) / 10000.f, static_cast<float>(packet->data.position.z) / 10000.f);
		XMFLOAT3 conversion_look = XMFLOAT3(static_cast<float>(packet->data.look.x) / 100.f, static_cast<float>(packet->data.look.y) / 100.f, static_cast<float>(packet->data.look.z) / 100.f);
		XMFLOAT3 conversion_right = XMFLOAT3(static_cast<float>(packet->data.right.x) / 100.f, static_cast<float>(packet->data.right.y) / 100.f, static_cast<float>(packet->data.right.z) / 100.f);

		Other_Player_Pos[i].pos_lock.lock();
		Other_Player_Pos[i].Other_Pos = conversion_position;
		m_ppOther[i]->SetIsColledUpFace(packet->data.is_collision_up_face);
		Other_Player_Pos[i].pos_lock.unlock();
		m_ppOther[i]->m_xmf3Look = conversion_look;
		m_ppOther[i]->m_xmf3Right = conversion_right;
		if (packet->data.input_key == DIR_FORWARD)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 1);
		}
		if (packet->data.input_key == DIR_BACKWARD)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 2);
		}
		if (packet->data.input_key == DIR_LEFT)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 3);
		}
		if (packet->data.input_key == DIR_RIGHT)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 4);
		}
		if (packet->data.input_key == DIR_UP)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 5);
		}

		if (packet->data.input_key == DIR_EMPTY)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 0);
		}

		if (packet->data.is_jump == true)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 6);
		}

		if (packet->data.is_victim == true)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 8);
		}

		if (packet->data.is_attack == true)
		{
			m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_ppOther[i]->SetTrackAnimationSet(0, 7);
		}
	}
}

void Network::Process_Other_Player_Move(char* ptr)
{
	sc_other_player_move* packet = reinterpret_cast<sc_other_player_move*>(ptr);
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			if (packet->data[i].id == m_ppOther[j]->GetID())
			{
				XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->data[i].position.x) / 10000.f, static_cast<float>(packet->data[i].position.y) / 10000.f, static_cast<float>(packet->data[i].position.z) / 10000.f);
				XMFLOAT3 conversion_look = XMFLOAT3(static_cast<float>(packet->data[i].look.x) / 100.f, static_cast<float>(packet->data[i].look.y) / 100.f, static_cast<float>(packet->data[i].look.z) / 100.f);
				XMFLOAT3 conversion_right = XMFLOAT3(static_cast<float>(packet->data[i].right.x) / 100.f, static_cast<float>(packet->data[i].right.y) / 100.f, static_cast<float>(packet->data[i].right.z) / 100.f);
				//m_ppOther[j]->SetPosition(conversion_position)
				Other_Player_Pos[j].pos_lock.lock();
				Other_Player_Pos[j].Other_Pos = conversion_position;
				m_ppOther[j]->SetIsColledUpFace(packet->data[i].is_collision_up_face);
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

				if (packet->data[i].is_jump == true)
				{
					m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					m_ppOther[j]->SetTrackAnimationSet(0, 6);
				}

				if (packet->data[i].is_victim == true)
				{
					m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					m_ppOther[j]->SetTrackAnimationSet(0, 8);
				}

				if (packet->data[i].is_attack == true)
				{
					m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					m_ppOther[j]->SetTrackAnimationSet(0, 7);
				}
			}
		}
	}
}

void Network::Process_Attack_Packet(char* ptr)
{
	sc_packet_attack* packet = reinterpret_cast<sc_packet_attack*>(ptr);

	if (packet->attacker_id == m_pPlayer->GetID())
	{
		// 굳이 할 필요 있음? 이미 애니메이션 동작하는데...ㅎ
	}


	for (int i = 0; i < 5; ++i)
	{
		if (m_ppOther[i]->GetID() == packet->attacker_id)
		{

		}
	}
}

void Network::Process_Door_Update(char* ptr)
{
	sc_packet_open_door* packet = reinterpret_cast<sc_packet_open_door*>(ptr);
	if (packet->door_state == 0)
		m_pDoors[static_cast<int>(packet->door_number)]->SetOpen(true);
	else if (packet->door_state == 2)
		m_pDoors[static_cast<int>(packet->door_number)]->SetOpen(false);
}

void Network::Process_ElectronicSystem_Reset_By_Tagger(char* ptr)
{
	sc_packet_request_electronic_system_reset* packet = reinterpret_cast<sc_packet_request_electronic_system_reset*>(ptr);

	for (int i = 0; i < 10; ++i)
		m_pPowers[packet->switch_index]->SetSwitchValue(i, false);
}

void Network::Process_ElectronicSystem_Reset_By_Player(char* ptr)
{
	sc_packet_request_electronic_system_reset* packet = reinterpret_cast<sc_packet_request_electronic_system_reset*>(ptr);

	for (int i = 0; i < 10; ++i)
		m_pPowers[packet->switch_index]->SetSwitchValue(i, false);
}

void Network::Process_ElectronicSystemDoor_Update(char* ptr)
{
	sc_packet_open_electronic_system_door* packet = reinterpret_cast<sc_packet_open_electronic_system_door*>(ptr);
	if (packet->es_state == 0)
		m_pPowers[packet->es_num]->SetOpen(true);
	else
		m_pPowers[packet->es_num]->SetOpen(false);
}

void Network::Process_ElectrinicSystem_Init(char* ptr)
{
	sc_packet_electronic_system_init* packet = reinterpret_cast<sc_packet_electronic_system_init*>(ptr);

	for (int i = 0; i < 5; ++i)
	{
		for (int idx = 0; idx < 10; ++idx)
		{
			m_pPowers[i]->SetAnswer(idx, packet->data[i].value);
			m_pPowers[i]->SetIndex(i);
		}
	}
}

void Network::Process_ElectronicSystem_Switch_Update(char* ptr)
{
	sc_packet_electronic_system_update_value* packet = reinterpret_cast< sc_packet_electronic_system_update_value*>(ptr);
	m_pPowers[packet->es_num]->SetSwitchValue(packet->es_switch_idx, packet->es_value);
}

void Network::Process_ElectronicSystem_Activate(char* ptr)
{
	sc_packet_electronic_system_activate_update* packet = reinterpret_cast<sc_packet_electronic_system_activate_update*>(ptr);
	packet->system_index;
	packet->activate;
}

void Network::Process_Pick_Item_Init(char* ptr)
{
	sc_packet_pick_item_init* packet = reinterpret_cast<sc_packet_pick_item_init*>(ptr);
	// 여기서 처리해야함

	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
	{
		std::cout << "Item [" << packet->data[i].item_box_index << "] Type : ";
		if (packet->data[i].item_type == GAME_ITEM::ITEM_LIFECHIP)
			std::cout << "ITEM_LIFECHIP" << std::endl;
		if (packet->data[i].item_type == GAME_ITEM::ITEM_DRILL)
			std::cout << "ITEM_DRILL" << std::endl;
		if (packet->data[i].item_type == GAME_ITEM::ITEM_HAMMER)
			std::cout << "ITEM_HAMMER" << std::endl;
		if (packet->data[i].item_type == GAME_ITEM::ITEM_NONE)
			std::cout << "ITEM_NONE" << std::endl;
		if (packet->data[i].item_type == GAME_ITEM::ITEM_PLIERS)
			std::cout << "ITEM_PLIERS" << std::endl;
		if (packet->data[i].item_type == GAME_ITEM::ITEM_WRENCH)
			std::cout << "ITEM_WRENCH" << std::endl;
	}
}

void Network::Process_Pick_Item_Update(char* ptr)
{

}

void Network::Process_Active_Altar(char* ptr)
{
	sc_packet_activate_altar* packet = reinterpret_cast<sc_packet_activate_altar*>(ptr);
	// 여기서 술래 재단을 활성화 해야함.
}

void Network::Process_Altar_LifeChip_Update(char* ptr)
{
	sc_packet_altar_lifechip_update* packet = reinterpret_cast<sc_packet_altar_lifechip_update*>(ptr);
	packet->lifechip_count; // 이게 현재 수집된 생명칩 갯수임
}