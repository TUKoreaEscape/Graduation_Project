#include "stdafx.h"
#include "Movement.h"
#include "Input.h"
#include "Network.h"
#include "Object.h"

void CommonMovement::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CommonMovement::update(float elapsedTime)
{
		UCHAR keyBuffer[256];
		DWORD dwDirection = 0;
		bool		m_emptyKey = false;
		float speed = Input::GetInstance()->speed;
		memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
		if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		{
			//m_pPlayer를 사용하여 움직이는 코드를 작성하면 된다.
			//std::cout << "w키" << std::endl;
			dwDirection |= DIR_FORWARD;
			gameObject->SetAnimation(1);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_FORWARD);
		}
		if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		{
			//std::cout << "s키" << std::endl;
			dwDirection |= DIR_BACKWARD;
			gameObject->SetAnimation(2);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_BACKWARD);
		}
		if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		{
			//std::cout << "a키" << std::endl;
			dwDirection |= DIR_LEFT;
			gameObject->SetAnimation(3);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_LEFT);
		}
		if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
		{
			//std::cout << "d키" << std::endl;
			dwDirection |= DIR_RIGHT;
			gameObject->SetAnimation(4);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_RIGHT);
		}
		if ((keyBuffer[VK_SPACE] & 0xF0&&!Input::GetInstance()->m_pPlayer->GetIsFalling()))
		{
			//std::cout << "스페이스바" << std::endl;
			Input::GetInstance()->m_pPlayer->SetIsFalling(true);
		}

		if ((Input::GetInstance()->m_pPlayer->IsJump() && Input::GetInstance()->m_pPlayer->GetIsFalling()))
		{
			//std::cout << "점프 on" << std::endl;
			dwDirection |= DIR_UP;
			gameObject->SetAnimation(5);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_UP);
			Input::GetInstance()->m_pPlayer->SetJumpTime(elapsedTime);
		}

		if (Input::GetInstance()->m_pPlayer->GetIsFalling())
		{
			gameObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.33f);

			gameObject->SetAnimation(6);
		}

		if (keyBuffer['f'] & 0xF0 || keyBuffer['F'] & 0xF0)
		{
			// 여기 건드세요 @우빈``
			int playerType = Input::GetInstance()->m_pPlayer->GetType();
			if (Input::GetInstance()->m_pPlayer->m_pNearDoor)
			{
				bool DoorState = Input::GetInstance()->m_pPlayer->m_pNearDoor->IsOpen;
				if (Input::GetInstance()->m_pPlayer->m_pNearDoor->GetIsWorking() == false)
					Input::GetInstance()->m_pPlayer->m_pNearDoor->Interaction(playerType);
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearInteractionObejct) {
				Input::GetInstance()->m_pPlayer->m_pNearInteractionObejct->Interaction(playerType);
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearVent) {
				Input::GetInstance()->m_pPlayer->m_pNearVent->Interaction(playerType);
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearItembox) {
				Input::GetInstance()->m_pPlayer->m_pNearItembox->Interaction(playerType);
			}
			if (Input::GetInstance()->m_pPlayer->m_pNearTaggers) {
				Input::GetInstance()->m_pPlayer->m_pNearTaggers->Interaction(playerType);
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
			Input::GetInstance()->m_pPlayer->PlayAttack(elapsedTime);
			gameObject->SetAnimation(7);
			
		}
		else if (m_emptyKey && !Input::GetInstance()->m_pPlayer->GetIsFalling())
		{
			gameObject->SetAnimation(0);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_EMPTY);
		}
}
