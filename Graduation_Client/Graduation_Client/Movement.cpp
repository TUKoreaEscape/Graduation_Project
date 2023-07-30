#include "stdafx.h"
#include "Movement.h"
#include "Input.h"
#include "Network.h"
#include "Object.h"
#include "Game_state.h"

void CommonMovement::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CommonMovement::update(float elapsedTime)
{
	if (GameState::GetInstance()->GetChatState()) return;
	if (GameState::GetInstance()->GetGameState() == SPECTATOR_GAME) return;
	UCHAR keyBuffer[256];
	DWORD dwDirection = 0;
	bool		m_emptyKey = false;
	bool bInteraction = false;
	float speed = Input::GetInstance()->speed;
	memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
	if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
	{
		//m_pPlayer를 사용하여 움직이는 코드를 작성하면 된다.
		//std::cout << "w키" << std::endl;
		dwDirection |= DIR_FORWARD;
		gameObject->SetAnimation(RUN_FWD);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_FORWARD);
	}
	if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
	{
		//std::cout << "s키" << std::endl;
		dwDirection |= DIR_BACKWARD;
		gameObject->SetAnimation(RUN_BWD);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_BACKWARD);
	}
	if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
	{
		//std::cout << "a키" << std::endl;
		dwDirection |= DIR_LEFT;
		gameObject->SetAnimation(RUN_LEFT);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_LEFT);
	}
	if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
	{
		//std::cout << "d키" << std::endl;
		dwDirection |= DIR_RIGHT;
		gameObject->SetAnimation(RUN_RIGHT);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_RIGHT);
	}
	if ((keyBuffer[VK_SPACE] & 0xF0 && !Input::GetInstance()->m_pPlayer->GetIsFalling()))
	{
		//std::cout << "스페이스바" << std::endl;
		Input::GetInstance()->m_pPlayer->SetIsFalling(true);
		Sound::GetInstance()->PlayPlayerEffectSound(Input::GetInstance()->m_pPlayer->m_nJumpSound, 5, 1.0f);
	}

	if ((Input::GetInstance()->m_pPlayer->IsJump() && Input::GetInstance()->m_pPlayer->GetIsFalling()))
	{
		//std::cout << "점프 on" << std::endl;
		dwDirection |= DIR_UP;
		gameObject->SetAnimation(JUMP);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_UP);
		Input::GetInstance()->m_pPlayer->SetJumpTime(elapsedTime);
	}

	if (Input::GetInstance()->m_pPlayer->GetIsFalling())
	{
		gameObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.33f);

		//gameObject->SetAnimation(JUMP_END);
	}

	if (keyBuffer['f'] & 0xF0 || keyBuffer['F'] & 0xF0)
	{
		// 여기 건드세요 @우빈``
		int playerType = Input::GetInstance()->m_pPlayer->GetType();
		if (Input::GetInstance()->m_pPlayer->m_pNearDoor)
		{
			bool DoorState = Input::GetInstance()->m_pPlayer->m_pNearDoor->IsOpen;
			if (Input::GetInstance()->m_pPlayer->m_pNearDoor->GetIsWorking() == false) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearDoor->Interaction(playerType);
				bInteraction = true;
			}
		}
		if (Input::GetInstance()->m_gamestate->GetGameState() == PLAYING_GAME) {
			if (Input::GetInstance()->m_pPlayer->m_pNearInteractionObejct) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearInteractionObejct->Interaction(playerType);
				bInteraction = true;
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearVent) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearVent->Interaction(playerType);
				bInteraction = true;
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearItembox) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearItembox->Interaction(playerType);
				bInteraction = true;
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearTaggers) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearTaggers->Interaction(playerType);
				bInteraction = true;
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearEscape) {
				gameObject->SetAnimation(INTERACTION);
				Input::GetInstance()->m_pPlayer->m_pNearEscape->Interaction(playerType);
				bInteraction = true;
			}
		}
	}

	if (keyBuffer['1'] & 0xF0)
	{
#if USE_NETWORK
		if (Input::GetInstance()->m_pPlayer->GetType() == TYPE_TAGGER && Input::GetInstance()->m_pPlayer->GetTaggerSkill(0) == true)
		{
			Network& network = *Network::GetInstance();
			network.Send_Use_Tagger_Skill(1);
			Input::GetInstance()->m_pPlayer->UseTaggerSkill(0);
		}
#endif

	}

	if (keyBuffer['2'] & 0xF0)
	{
#if USE_NETWORK
		if (Input::GetInstance()->m_pPlayer->GetType() == TYPE_TAGGER && Input::GetInstance()->m_pPlayer->GetTaggerSkill(1) == true)
		{
			Network& network = *Network::GetInstance();
			network.Send_Use_Tagger_Skill(2);
			Input::GetInstance()->m_pPlayer->UseTaggerSkill(1);
		}
#endif
	}

	if (keyBuffer['3'] & 0xF0)
	{
#if USE_NETWORK
		if (Input::GetInstance()->m_pPlayer->GetType() == TYPE_TAGGER && Input::GetInstance()->m_pPlayer->GetTaggerSkill(2) == true)
		{
			Network& network = *Network::GetInstance();
			network.Send_Use_Tagger_Skill(3);
			Input::GetInstance()->m_pPlayer->UseTaggerSkill(2);
		}
#endif
	}

	if (dwDirection)
	{
		if ((dwDirection == DIR_NOT_FB) ||
			(dwDirection == DIR_NOT_LR) ||
			(dwDirection == DIR_NOT_FBLR))
			m_emptyKey = true;
		//std::cout << elapsedTime << std::endl;
		Input::GetInstance()->m_pPlayer->Move(dwDirection, speed * elapsedTime, true);
	}
	else m_emptyKey = true;

	if (Input::GetInstance()->m_pPlayer->IsAttack())
	{
		return;
		//gameObject->SetAnimation(ATTACK);

	}
	else if (m_emptyKey && !Input::GetInstance()->m_pPlayer->GetIsFalling())
	{
		if (bInteraction)
			gameObject->SetAnimation(INTERACTION);
		else 
			gameObject->SetAnimation(IDLE);
		Input::GetInstance()->m_pPlayer->SetDirection(DIR_EMPTY);
	}
}
