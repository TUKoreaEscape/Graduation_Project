#include "Network.h"
#include "Object.h"
#include "GameObject.h"
#include "Input.h"

void Network::Process_PlayerRate(char* ptr)
{
    sc_packet_player_rate* packet = reinterpret_cast<sc_packet_player_rate*>(ptr);

    std::cout << "============= 플레이어 레이팅 정보 =============" << std::endl;
    std::cout << "총 플레이 횟수 : " << packet->total_play << std::endl;
    std::cout << "술래 플레이 횟수 : " << packet->tagger_play;
    if (packet->tagger_play != 0)
        std::cout << " || [승률 : " << static_cast<float>(static_cast<float>(packet->tagger_win) / static_cast<float>(packet->tagger_play)) * 100 << "%]" << std::endl;
    else
        std::cout << " || [승률 : 0%]" << std::endl;

    std::cout << "생존자 플레이 횟수 : " << packet->runner_play;
    if (packet->runner_play != 0)
        std::cout << " || [승률 : " << static_cast<float>(static_cast<float>(packet->runner_win) / static_cast<float>(packet->runner_play)) * 100 << "%]" << std::endl;
    else
        std::cout << " || [승률 : 0%]" << std::endl;
    std::cout << "=======================================" << std::endl;
}

void Network::Process_Player_Exit(char* ptr)
{
    sc_packet_player_exit* packet = reinterpret_cast<sc_packet_player_exit*>(ptr);
    for (int i = 0; i < 5; ++i)
    {
        if (m_ppOther[i]->GetID() == -1)
            continue;
        if (m_ppOther[i]->GetID() == packet->user_id) {
            m_ppOther[i]->SetID(-1);
            if (GameState::GetInstance()->GetGameState() == PLAYING_GAME) // 게임도중 사라진 경우 아예 제거목적
                m_ppOther[i]->SetPosition(XMFLOAT3(-5000, -5000, -5000));
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
            set_capture_mouse();
            m_pPlayer->SetPosition(packet->position[i], true);
            m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
            m_pPlayer->SetTrackAnimationSet(0, 0);
            GameState& game_state = *GameState::GetInstance();
            game_state.ChangeNextState();
            break;
        }
    }

    for (int i = 0; i < 5; ++i) {
        set_capture_mouse();
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

    for (int i = 0; i < 3; ++i) {
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[i])->SetID(i);
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[i])->SetActivate(false);
    }

    for (auto& door_object : m_pDoors)
        door_object->SetOpen(false);
}

void Network::Process_Chat(char* ptr)
{
    sc_packet_chat* packet = reinterpret_cast<sc_packet_chat*>(ptr);
    Input::GetInstance()->Receive(packet->message, packet->name);
}

void Network::Process_Game_Start(char* ptr)
{
    sc_packet_game_start* packet = reinterpret_cast<sc_packet_game_start*>(ptr);
    GameState& game_state = *GameState::GetInstance();
    game_state.SetLoading(0.0f);
    set_capture_mouse();
}

void Network::Process_Game_End(char* ptr)
{
    Input::GetInstance()->speed = 60.f;
    m_pPlayer->SetInfo(NOINFO);
    release_capture_mouse();
    sc_packet_game_end* packet = reinterpret_cast<sc_packet_game_end*>(ptr);
    m_tagger_win = packet->is_tagger_win;

    for (int i = 0; i < 6; ++i)
        m_escape_player_id[i] = -1;

    for (int i = 0; i < 6; ++i)
        m_escape_player_id[i] = packet->escape_id[i]; // 탈출한 사람 id를 저장함

    for (int i = 0; i < 5; ++i)
        m_other_player_ready[i] = false;

    for (int i = 0; i < NUM_ESCAPE_LEVER; ++i)
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[i])->Init_By_GameEnd();

    reinterpret_cast<TaggersBox*>(m_Taggers_Box)->Reset();

    GameState& game_state = *GameState::GetInstance();

    if (game_state.GetGameState() == SPECTATOR_GAME || game_state.GetGameState() == INTERACTION_POWER)
        game_state.ChangeNextState();
    else { // 탈출을 못하여 PLAYING GAME 상태이거나 술래가 승리한 경우 STATE를 2번 바꿔야 GAME_END로 넘어가짐
        for (int i = 0; i < 2; ++i)
            game_state.ChangeNextState();
    }

    for (int i = 0; i < MAX_INGAME_ITEM; ++i) {
        m_pBoxes[i]->SetOpen(false);
        m_pBoxes[i]->SetUnBlock();
    }
    for (int i = 0; i < 6; ++i) {
        m_pDoors[i]->SetOpen(false);
        m_pDoors[i]->SetUnBlock();
    }
    for (int i = 0; i < 5; ++i) {
        m_pPowers[i]->SetOpen(false);
        m_pPowers[i]->SetUnBlock();
    }
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

    for (int i = 0; i < 5; ++i) {
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
    pos_lock.lock();
    m_pPlayer_Pos = conversion_position;
    m_pPlayer->SetIsColledUpFace(packet->is_collision_up_face);
    pos_lock.unlock();
}

void Network::Process_Other_Move(char* ptr)
{
    sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);

    //for (int i = 0; i < 5; ++i)
    //{
    //   if (packet->data.id != m_ppOther[i]->GetID())
    //      continue;
    //   XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->data.position.x) / 10000.f, static_cast<float>(packet->data.position.y) / 10000.f, static_cast<float>(packet->data.position.z) / 10000.f);
    //   XMFLOAT3 conversion_look = XMFLOAT3(static_cast<float>(packet->data.look.x) / 100.f, static_cast<float>(packet->data.look.y) / 100.f, static_cast<float>(packet->data.look.z) / 100.f);
    //   XMFLOAT3 conversion_right = XMFLOAT3(static_cast<float>(packet->data.right.x) / 100.f, static_cast<float>(packet->data.right.y) / 100.f, static_cast<float>(packet->data.right.z) / 100.f);

    //   Other_Player_Pos[i].pos_lock.lock();
    //   Other_Player_Pos[i].Other_Pos = conversion_position;
    //   m_ppOther[i]->SetIsColledUpFace(packet->data.is_collision_up_face);
    //   m_ppOther[i]->m_xmf3Look = conversion_look;
    //   m_ppOther[i]->m_xmf3Right = conversion_right;
    //   Other_Player_Pos[i].pos_lock.unlock();
    //   if (packet->data.input_key == DIR_FORWARD)
    //   {
    //      if (m_before_animation_index[i] != 1) {
    //         m_ppOther[i]->SetAnimation(1);
    //         m_before_animation_index[i] = 1;
    //      }
    //   }
    //   if (packet->data.input_key == DIR_BACKWARD)
    //   {
    //      //m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
    //      if (m_before_animation_index[i] != 2) {
    //         m_ppOther[i]->SetAnimation(2);
    //         m_before_animation_index[i] = 2;
    //      }
    //   }
    //   if (packet->data.input_key == DIR_LEFT)
    //   {
    //      //m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
    //      if (m_before_animation_index[i] != 3) {
    //         m_ppOther[i]->SetAnimation(3);
    //         m_before_animation_index[i] = 3;
    //      }
    //   }
    //   if (packet->data.input_key == DIR_RIGHT)
    //   {
    //      //m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
    //      if (m_before_animation_index[i] != 4) {
    //         m_ppOther[i]->SetAnimation(4);
    //         m_before_animation_index[i] = 4;
    //      }
    //   }
    //   if (packet->data.input_key == DIR_UP)
    //   {
    //      //m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
    //      if (m_before_animation_index[i] != 5) {
    //         m_ppOther[i]->SetAnimation(5);
    //         m_before_animation_index[i] = 5;
    //      }
    //   }

    //   if (packet->data.input_key == DIR_EMPTY)
    //   {
    //      //m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
    //      if (m_before_animation_index[i] != 0) {
    //         m_ppOther[i]->SetAnimation(0);
    //         m_before_animation_index[i] = 0;
    //      }
    //   }

    //   if (packet->data.is_jump == true)
    //   {
    //      if (m_before_animation_index[i] != 6) {
    //         m_ppOther[i]->SetAnimation(6);
    //         m_before_animation_index[i] = 6;
    //      }
    //   }

    //   if (packet->data.is_victim == true)
    //   {
    //      if (m_before_animation_index[i] != 8) {
    //         m_ppOther[i]->SetAnimation(8);
    //         m_before_animation_index[i] = 8;
    //      }
    //   }

    //   if (packet->data.is_attack == true)
    //   {
    //      if (m_before_animation_index[i] != 7) {
    //         m_ppOther[i]->SetAnimation(7);
    //         m_before_animation_index[i] = 7;
    //      }
    //   }
    //}

    for (int j = 0; j < 5; ++j)
    {
        if (packet->data.id != m_ppOther[j]->GetID())
            continue;
        if (packet->data.id == m_ppOther[j]->GetID())
        {
            XMFLOAT3 conversion_position = XMFLOAT3(static_cast<float>(packet->data.position.x) / 10000.f, static_cast<float>(packet->data.position.y) / 10000.f, static_cast<float>(packet->data.position.z) / 10000.f);
            XMFLOAT3 conversion_look = XMFLOAT3(static_cast<float>(packet->data.look.x) / 100.f, static_cast<float>(packet->data.look.y) / 100.f, static_cast<float>(packet->data.look.z) / 100.f);
            XMFLOAT3 conversion_right = XMFLOAT3(static_cast<float>(packet->data.right.x) / 100.f, static_cast<float>(packet->data.right.y) / 100.f, static_cast<float>(packet->data.right.z) / 100.f);
            //m_ppOther[j]->SetPosition(conversion_position)
            Other_Player_Pos[j].pos_lock.lock();
            Other_Player_Pos[j].Other_Pos = conversion_position;
            m_ppOther[j]->SetIsColledUpFace(packet->data.is_collision_up_face);
            Other_Player_Pos[j].pos_lock.unlock();
            m_ppOther[j]->m_xmf3Look = conversion_look;
            m_ppOther[j]->m_xmf3Right = conversion_right;
            if (packet->data.input_key == DIR_FORWARD)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 1);
            }
            if (packet->data.input_key == DIR_BACKWARD)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 2);
            }
            if (packet->data.input_key == DIR_LEFT)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 3);
            }
            if (packet->data.input_key == DIR_RIGHT)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 4);
            }
            if (packet->data.input_key == DIR_UP)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 5);
            }

            if (packet->data.input_key == DIR_EMPTY)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 0);
            }

            if (packet->data.is_jump == true)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                //m_ppOther[j]->SetTrackAnimationSet(0, 6);
            }

            if (packet->data.is_victim == true)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetTrackAnimationSet(0, 8);
            }

            if (packet->data.is_attack == true)
            {
                m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                m_ppOther[j]->SetAttackZeroTime(j);
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
                    //m_ppOther[j]->SetTrackAnimationSet(0, 6);
                }

                if (packet->data[i].is_victim == true)
                {
                    m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                    m_ppOther[j]->SetTrackAnimationSet(0, 8);
                }

                if (packet->data[i].is_attack == true)
                {
                    m_ppOther[j]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
                    m_ppOther[j]->SetAttackZeroTime(j);
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

void Network::Process_ElectonicSystem_Lever_Update(char* ptr)
{
    sc_packet_electronic_system_lever_working* packet = reinterpret_cast<sc_packet_electronic_system_lever_working*>(ptr);

    if (packet->is_start)
        reinterpret_cast<PowerSwitch*>(m_pPowers[packet->index])->CheckStart();
    else
        reinterpret_cast<PowerSwitch*>(m_pPowers[packet->index])->CheckStop();
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
        //std::cout << i << " : ";
        m_pPowers[i]->SetIndex(i);
        m_pPowers[i]->SetActivate(false);
        m_pPowers[i]->SetOpen(false);
        for (int idx = 0; idx < 10; ++idx)
        {
            //std::cout << static_cast<int>(packet->data[i].value[idx]) << " ";
            m_pPowers[i]->SetAnswer(idx, packet->data[i].value[idx]);
            m_pPowers[i]->SetSwitchValue(idx, false);
        }
        //std::cout << std::endl;
    }
}

void Network::Process_ElectronicSystem_Switch_Update(char* ptr)
{
    sc_packet_electronic_system_update_value* packet = reinterpret_cast<sc_packet_electronic_system_update_value*>(ptr);
    m_pPowers[packet->es_num]->SetSwitchValue(packet->es_switch_idx, packet->es_value);
}

void Network::Process_ElectronicSystem_Activate(char* ptr)
{
    sc_packet_electronic_system_activate_update* packet = reinterpret_cast<sc_packet_electronic_system_activate_update*>(ptr);
    m_pPowers[packet->system_index]->CheckStop();
    m_pPowers[packet->system_index]->SetActivate(packet->activate);
    //if (packet->activate)
    //   std::cout << packet->system_index << "번 전력장치 수리 완료" << std::endl;
}

void Network::Process_Pick_Item_Init(char* ptr)
{
    sc_packet_pick_item_init* packet = reinterpret_cast<sc_packet_pick_item_init*>(ptr);
    // 여기서 처리해야함
    for (int i = 0; i < 5; ++i)
        m_item_to_power[i] = packet->shuffle[i];

    //for (int i = 0; i < 5; ++i)
    //   std::cout << m_item_to_power[i] << " ";
    //std::cout << std::endl;

    for (int i = 0; i < MAX_INGAME_ITEM; ++i) {
        m_pBoxes[i]->SetIndex(i);
        m_pBoxes[i]->SetOpen(false);
    }

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
            m_pPlayer->SetPlayerType(TYPE_PLAYER);
        else
            m_pPlayer->m_got_item = packet->item_type;
    }

    else {
        if (packet->item_type != GAME_ITEM::ITEM_LIFECHIP)
            return;

        for (int i = 0; i < 5; ++i) {
            if (packet->own_id == m_ppOther[i]->GetID())
                m_ppOther[i]->SetPlayerType(TYPE_PLAYER);
        }
    }
}

void Network::Process_Active_Altar(char* ptr)
{
    sc_packet_activate_altar* packet = reinterpret_cast<sc_packet_activate_altar*>(ptr);
    // 여기서 술래 재단을 활성화 해야함.
    m_Taggers_Box;
    m_pPlayer->SetInfo(ACTIVATE_ALTAR);
    std::cout << "술래가 재단을 활성화 하였습니다" << std::endl;
}

void Network::Process_Active_EscapeSystem(char* ptr)
{
    sc_packet_escapesystem_activate* packet = reinterpret_cast<sc_packet_escapesystem_activate*>(ptr);
    //std::cout << "탈출장치 [" << packet->index << "]번 활성화" << std::endl;

    for (int i = 0; i < NUM_ESCAPE_LEVER; ++i)
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[i])->SetWorking();
    reinterpret_cast<EscapeObject*>(m_EscapeLevers[packet->index])->SetReal();
    m_pPlayer->SetInfo(ACTIVATE_ESCAPESYSTEM);
}

void Network::Process_EscapeSystem_Lever_Update(char* ptr)
{
    sc_packet_request_escapesystem_lever_working* packet = reinterpret_cast<sc_packet_request_escapesystem_lever_working*>(ptr);

    if (packet->is_start)
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[packet->index])->CheckStart();
    else
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[packet->index])->CheckStop();
}

void Network::Process_EscapeSystem_Update(char* ptr)
{
    sc_packet_request_escapesystem_working* packet = reinterpret_cast<sc_packet_request_escapesystem_working*>(ptr);
    packet->index; // 해당 번호의 탈출장치 working시 뭔가 조작을 할 예정인데 이건 추후 예정

    if (m_pPlayer->GetType() == TYPE_TAGGER)
        m_pPlayer->SetInfo(ESCAPE_SUCCESS);
    else {
        if (packet->info)
            m_pPlayer->SetInfo(WORKING_ESCAPESYSTEM);
    }

    if (m_pPlayer->GetID() == packet->escape_id) {
        m_pPlayer->SetPlayerType(TYPE_ESCAPE_PLAYER); // 이후 ESCAPE_PLAYER로 교체해야함
        reinterpret_cast<EscapeObject*>(m_EscapeLevers[packet->index])->CheckStop();
        GameState& game_state = *GameState::GetInstance();
        game_state.ChangeNextState();
    }
    else {
        for (int i = 0; i < 5; ++i) {
            if (m_ppOther[i]->GetID() == packet->escape_id) {
                reinterpret_cast<EscapeObject*>(m_EscapeLevers[packet->index])->CheckStop();
                m_ppOther[i]->SetPlayerType(TYPE_ESCAPE_PLAYER); // 이후 ESCAPE_PLAYER로 교체해야함
                break;
            }
        }
    }
}

void Network::Process_Altar_LifeChip_Update(char* ptr)
{
    sc_packet_altar_lifechip_update* packet = reinterpret_cast<sc_packet_altar_lifechip_update*>(ptr);
    packet->lifechip_count; // 이게 현재 수집된 생명칩 갯수임
    reinterpret_cast<TaggersBox*>(m_Taggers_Box)->CollectChip();
}

void Network::Process_Activate_Tagger_Skill(char* ptr)
{
    sc_packet_tagger_skill* packet = reinterpret_cast<sc_packet_tagger_skill*>(ptr);

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