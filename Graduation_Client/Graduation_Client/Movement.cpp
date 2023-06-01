#include "stdafx.h"
#include "Movement.h"
#include "Input.h"
#include "Network.h"

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
			gameObject->SetTrackAnimationSet(0, 1);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_FORWARD);
		}
		if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		{
			//std::cout << "s키" << std::endl;
			dwDirection |= DIR_BACKWARD;
			gameObject->SetTrackAnimationSet(0, 2);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_BACKWARD);
		}
		if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		{
			//std::cout << "a키" << std::endl;
			dwDirection |= DIR_LEFT;
			gameObject->SetTrackAnimationSet(0, 3);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_LEFT);
		}
		if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
		{
			//std::cout << "d키" << std::endl;
			dwDirection |= DIR_RIGHT;
			gameObject->SetTrackAnimationSet(0, 4);
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
			gameObject->SetTrackAnimationSet(0, 5);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_UP);
			Input::GetInstance()->m_pPlayer->SetJumpTime(elapsedTime);
		}

		if (Input::GetInstance()->m_pPlayer->GetIsFalling())
		{
			gameObject->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.33f);
			gameObject->SetTrackAnimationSet(0, 6);
		}

		if (keyBuffer['f'] & 0xF0 || keyBuffer['F'] & 0xF0)
		{
			// 여기 건드세요 @우빈``
			if (Input::GetInstance()->m_pPlayer->m_pNearDoor)
			{
				bool DoorState = Input::GetInstance()->m_pPlayer->m_pNearDoor->IsOpen;
				if (Input::GetInstance()->m_pPlayer->m_pNearDoor->GetIsWorking() == false)
				{
#if !USE_NETWORK
					Input::GetInstance()->m_pPlayer->m_pNearDoor->SetOpen(!DoorState);
#endif
#if USE_NETWORK
					cs_packet_request_open_door packet;
					packet.size = sizeof(packet);
					packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
					packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;

					Network& network = *Network::GetInstance();
					network.send_packet(&packet);
#endif
				}
			}
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
			gameObject->SetTrackAnimationSet(0, 7);
			
		}
		else if (m_emptyKey && !Input::GetInstance()->m_pPlayer->GetIsFalling())
		{
			gameObject->SetTrackAnimationSet(0, 0);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_EMPTY);
		}
}
