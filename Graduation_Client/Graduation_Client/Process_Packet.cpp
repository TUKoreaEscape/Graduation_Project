#include "Network.h"
#include "Object.h"
#include "GameObject.h"

void Network::Process_Player_Exit(char* ptr)
{
	sc_packet_player_exit* packet = reinterpret_cast<sc_packet_player_exit*>(ptr);
	std::cout << "나간패킷 받음" << std::endl;
	for (int i = 0; i < 5; ++i)
	{
		if (m_ppOther[i]->GetID() == -1)
			continue;
		if (m_ppOther[i]->GetID() == packet->user_id) {
			m_ppOther[i]->SetID(-1);
		}
	}
}

void Network::Process_Ready(char* ptr)
{
	sc_packet_ready* packet = reinterpret_cast<sc_packet_ready*>(ptr);

	for (int i = 0; i < 5; ++i) {
		if (m_ppOther[i]->GetID() == -1)
			continue;
		if (m_ppOther[i]->GetID() == packet->id) {
			if (packet->ready_type) {
				m_other_player_ready[i] = true;
				m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
				m_ppOther[i]->SetTrackAnimationSet(0, 9);
			}
			else {
				m_other_player_ready[i] = false;
				m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
				m_ppOther[i]->SetTrackAnimationSet(0, 0);
			}
		}
	}
}

void Network::Process_Init_Position(char* ptr)
{
	sc_packet_init_position* packet = reinterpret_cast<sc_packet_init_position*>(ptr);

	for (int i = 0; i < 6; ++i) {
		if (m_pPlayer->GetID() == packet->user_id[i]) {
			m_pPlayer->SetPosition(packet->position[i], true);
			m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			m_pPlayer->SetTrackAnimationSet(0, 0);
			GameState& game_state = *GameState::GetInstance();
			game_state.ChangeNextState();
			break;
		}
	}

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 6; ++j) {
			if (m_ppOther[i]->GetID() == packet->user_id[j]) {
				m_ppOther[i]->SetPosition(packet->position[j], true);
				m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
				m_ppOther[i]->SetTrackAnimationSet(0, 0);
			}
		}
	}
	m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
	for (int i = 0; i < 5; ++i)
		m_ppOther[i]->SetPlayerType(TYPE_PLAYER_YET);
}

void Network::Process_Game_Start(char* ptr)
{
	sc_packet_game_start* packet = reinterpret_cast<sc_packet_game_start*>(ptr);
	GameState& game_state = *GameState::GetInstance();
	game_state.SetLoading(0.0f);
}

void Network::Process_Game_End(char* ptr)
{
	sc_packet_game_end* packet = reinterpret_cast<sc_packet_game_end*>(ptr);
	m_tagger_win = packet->is_tagger_win;

	for (int i = 0; i < 5; ++i)
		m_other_player_ready[i] = false;

	GameState& game_state = *GameState::GetInstance();
	game_state.ChangeNextState();

	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
		m_pBoxes[i]->SetOpen(false);
	for (int i = 0; i < 6; ++i)
		m_pDoors[i]->SetOpen(false);
	for (int i = 0; i < 5; ++i)
		m_pPowers[i]->SetOpen(false);
	for (int i = 0; i < 8; ++i) {
		m_Vents[i]->SetOpen(false);
		reinterpret_cast<Vent*>(m_Vents[i])->SetUnBlock();
	}
	m_lifechip = false;
	m_pPlayer->m_got_item = GAME_ITEM::ITEM_NONE;
}

void Network::Process_LifeChip_Update(char* ptr)
{
	sc_packet_life_chip_update* packet = reinterpret_cast<sc_packet_life_chip_update*>(ptr);

	if (m_pPlayer->GetID() == packet->id) {
		if (false == packet->life_chip)
			m_pPlayer->SetPlayerType(TYPE_DEAD_PLAYER);
		else
			m_pPlayer->SetPlayerType(TYPE_PLAYER);
		return;
	}

	for (int i = 0; i < 5; ++i){
		if (m_ppOther[i]->GetID() == packet->id) {
			if (false == packet->life_chip)
				m_ppOther[i]->SetPlayerType(TYPE_DEAD_PLAYER);
			else
				m_ppOther[i]->SetPlayerType(TYPE_PLAYER);
			return;
		}
	}
}

void Network::Process_Tagger_Collect_LifeChip(char* ptr)
{
	sc_packet_tagger_correct_life_chip* packet = reinterpret_cast<sc_packet_tagger_correct_life_chip*>(ptr);
	if (packet->life_chip == true) {
		reinterpret_cast<IngameUI*>(m_UIPlay[1])->SetGuage(1.0f);
		m_lifechip = true;
	}
	else {
		reinterpret_cast<IngameUI*>(m_UIPlay[1])->SetGuage(-1.0f);
		m_lifechip = false;
	}
}

void Network::Process_Player_Move(char* ptr)
{
	m_pPlayer_before_Pos = m_pPlayer_Pos;
	sc_packet_calculate_move* packet = reinterpret_cast<sc_packet_calculate_move*>(ptr);
	//m_pPlayer->SetPosition(packet->pos);	
	XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->pos.x) / 10000.f, static_cast<float>(packet->pos.y) / 10000.f, static_cast<float>(packet->pos.z) / 10000.f);
	//pos_lock.lock();
	m_pPlayer_Pos = conversion_position;
	m_pPlayer->SetIsColledUpFace(packet->is_collision_up_face);
	//pos_lock.unlock();
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

		//Other_Player_Pos[i].pos_lock.lock();
		Other_Player_Pos[i].Other_Pos = conversion_position;
		m_ppOther[i]->SetIsColledUpFace(packet->data.is_collision_up_face);
		//Other_Player_Pos[i].pos_lock.unlock();
		m_ppOther[i]->m_xmf3Look = conversion_look;
		m_ppOther[i]->m_xmf3Right = conversion_right;
		if (packet->data.input_key == DIR_FORWARD)
		{
			if (m_before_animation_index[i] != 1) {
				m_ppOther[i]->SetAnimation(1);
				m_before_animation_index[i] = 1;
			}
		}
		if (packet->data.input_key == DIR_BACKWARD)
		{
			//m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			if (m_before_animation_index[i] != 2) {
				m_ppOther[i]->SetAnimation(2);
				m_before_animation_index[i] = 2;
			}
		}
		if (packet->data.input_key == DIR_LEFT)
		{
			//m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			if (m_before_animation_index[i] != 3) {
				m_ppOther[i]->SetAnimation(3);
				m_before_animation_index[i] = 3;
			}
		}
		if (packet->data.input_key == DIR_RIGHT)
		{
			//m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			if (m_before_animation_index[i] != 4) {
				m_ppOther[i]->SetAnimation(4);
				m_before_animation_index[i] = 4;
			}
		}
		if (packet->data.input_key == DIR_UP)
		{
			//m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			if (m_before_animation_index[i] != 5) {
				m_ppOther[i]->SetAnimation(5);
				m_before_animation_index[i] = 5;
			}
		}

		if (packet->data.input_key == DIR_EMPTY)
		{
			//m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
			if (m_before_animation_index[i] != 0) {
				m_ppOther[i]->SetAnimation(0);
				m_before_animation_index[i] = 0;
			}
		}

		if (packet->data.is_jump == true)
		{
			if (m_before_animation_index[i] != 6) {
				m_ppOther[i]->SetAnimation(6);
				m_before_animation_index[i] = 6;
			}
		}

		if (packet->data.is_victim == true)
		{
			if (m_before_animation_index[i] != 8) {
				m_ppOther[i]->SetAnimation(8);
				m_before_animation_index[i] = 8;
			}
		}

		if (packet->data.is_attack == true)
		{
			if (m_before_animation_index[i] != 7) {
				m_ppOther[i]->SetAnimation(7);
				m_before_animation_index[i] = 7;
			}
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

void Network::Process_Hidden_Door_Update(char* ptr)
{
	sc_packet_open_hidden_door* packet = reinterpret_cast<sc_packet_open_hidden_door*>(ptr);
	if (packet->door_state == 0)
		m_Vents[static_cast<int>(packet->door_num)]->SetOpen(true);
	else if (packet->door_state == 2)
		m_Vents[static_cast<int>(packet->door_num)]->SetOpen(false);
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
		m_pPowers[i]->SetIndex(i);
		m_pPowers[i]->SetActivate(false);
		for (int idx = 0; idx < 10; ++idx)
		{
			m_pPowers[i]->SetAnswer(idx, packet->data[i].value[idx]);
			m_pPowers[i]->SetSwitchValue(idx, false);
		}
		std::cout << std::endl;
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
	m_pPowers[packet->system_index]->SetActivate(packet->activate);
	if (packet->activate)
		std::cout << packet->system_index << "번 전력장치 수리 완료" << std::endl;
}

void Network::Process_Pick_Item_Init(char* ptr)
{
	sc_packet_pick_item_init* packet = reinterpret_cast<sc_packet_pick_item_init*>(ptr);
	// 여기서 처리해야함

	for(int i = 0; i < MAX_INGAME_ITEM; ++i)
		m_pBoxes[i]->SetIndex(i);

	for (int i = 0; i < MAX_INGAME_ITEM; ++i)
	{
		for (int idx = 0; idx < MAX_INGAME_ITEM; ++idx)
		{
			if (m_pBoxes[i]->m_item_box_index == packet->data[idx].item_box_index) {
				m_pBoxes[i]->SetItem(packet->data[idx].item_type);
				break;
			}
		}
	}
}

void Network::Process_Pick_Item_Box_Update(char* ptr)
{
	sc_packet_item_box_update* packet = reinterpret_cast<sc_packet_item_box_update*>(ptr);
	m_pBoxes[packet->box_index]->SetOpen(packet->is_open);
}

void Network::Process_Pick_Item_Update(char* ptr)
{
	sc_packet_pick_fix_item_update* packet = reinterpret_cast<sc_packet_pick_fix_item_update*>(ptr);
	
	if (packet->item_type == GAME_ITEM::ITEM_NONE)
		return;

	m_pBoxes[packet->box_index]->m_item = GAME_ITEM::ITEM_NONE;
	
	if (packet->own_id == m_pPlayer->GetID()) {
		if (packet->item_type == GAME_ITEM::ITEM_LIFECHIP)
			m_pPlayer->SetType(TYPE_PLAYER);
		else
			m_pPlayer->m_got_item = packet->item_type;
	}

	else {
		if (packet->item_type != GAME_ITEM::ITEM_LIFECHIP)
			return;

		for (int i = 0; i < 5; ++i) {
			if (packet->own_id == m_ppOther[i]->GetID())
				m_ppOther[i]->SetType(TYPE_PLAYER);
		}
	}
}

void Network::Process_Active_Altar(char* ptr)
{
	sc_packet_activate_altar* packet = reinterpret_cast<sc_packet_activate_altar*>(ptr);
	// 여기서 술래 재단을 활성화 해야함.
	m_Taggers_Box;
	std::cout << "술래가 재단을 활성화 하였습니다" << std::endl;
}

void Network::Process_Altar_LifeChip_Update(char* ptr)
{
	sc_packet_altar_lifechip_update* packet = reinterpret_cast<sc_packet_altar_lifechip_update*>(ptr);
	packet->lifechip_count; // 이게 현재 수집된 생명칩 갯수임
	std::cout << "현재 재단에 모인 생명칩 갯수 : " << packet->lifechip_count << std::endl;
}

void Network::Process_Activate_Tagger_Skill(char* ptr)
{
	sc_packet_tagger_skill* packet = reinterpret_cast<sc_packet_tagger_skill*>(ptr);

	std::cout << "first skill : " << packet->first_skill << std::endl;
	std::cout << "second_skill : " << packet->second_skill << std::endl;
	std::cout << "third_skill : " << packet->third_skill << std::endl;
	if (packet->first_skill)
		m_pPlayer->SetTaggerSkill(0);
	if (packet->second_skill)
		m_pPlayer->SetTaggerSkill(1);
	if (packet->third_skill)
		m_pPlayer->SetTaggerSkill(2);
}

void Network::Process_Use_First_Tagger_Skill(char* ptr)
{
	sc_packet_use_first_tagger_skill* packet = reinterpret_cast<sc_packet_use_first_tagger_skill*>(ptr);
	
	for (int i = 0; i < 5; ++i) {
		if (packet->electronic_system_close[i])
			m_pPowers[i]->SetOpen(false);
		else
			m_pPowers[i]->SetOpen(true);
	}
}

void Network::Process_Use_Second_Tagger_Skill(char* ptr)
{
	sc_packet_use_second_tagger_skill* packet = reinterpret_cast<sc_packet_use_second_tagger_skill*>(ptr);

	if (packet->is_start) {
		for (int i = 0; i < 6; ++i)
			m_pDoors[i]->SetBlock();
	}
	else {
		for (int i = 0; i < 6; ++i)
			m_pDoors[i]->SetUnBlock();
	}
}

void Network::Process_Use_Third_Tagger_Skill(char* ptr)
{
	sc_packet_use_third_tagger_skill* packet = reinterpret_cast<sc_packet_use_third_tagger_skill*>(ptr);
	reinterpret_cast<Vent*>(m_Vents[packet->unactivate_vent])->SetBlock();
}