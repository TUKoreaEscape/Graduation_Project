#include "Network.h"

void Network::Process_Game_Start(char* ptr)
{
	sc_packet_game_start* packet = reinterpret_cast<sc_packet_game_start*>(ptr);

	// 여기서 로비씬 -> 게임씬으로 전환 해주면 됨
}

void Network::Process_Player_Move(char* ptr)
{
	sc_packet_calculate_move* packet = reinterpret_cast<sc_packet_calculate_move*>(ptr);
	//m_pPlayer->SetPosition(packet->pos);
	
	
	
	XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->pos.x) / 10000.f, static_cast<float>(packet->pos.y) / 10000.f, static_cast<float>(packet->pos.z) / 10000.f);
	pos_lock.lock();
	m_pPlayer_Pos = conversion_position;
	m_pPlayer->SetIsColledUpFace(packet->is_collision_up_face);
	pos_lock.unlock();
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
					m_ppOther[j]->SetTrackAnimationSet(0, 5);
				}
			}
		}
	}
}