﻿#include "stdafx.h"
#include "Movement.h"
#include "Input.h"

void CommonMovement::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CommonMovement::update(float elapsedTime)
{
		UCHAR keyBuffer[256];
		DWORD dwDirection = 0;
		memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
		if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		{
			//m_pPlayer를 사용하여 움직이는 코드를 작성하면 된다.
			std::cout << "w키" << std::endl;
			//gameObject->m_xmf4x4ToParent._43 += 0.005f;
			dwDirection |= DIR_FORWARD;
			gameObject->SetAnimationSet(1);
		}
		if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		{
			std::cout << "s키" << std::endl;
			//gameObject->m_xmf4x4ToParent._43 -= 0.005f;
			dwDirection |= DIR_BACKWARD;
			gameObject->SetAnimationSet(2);
		}
		if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		{
			std::cout << "a키" << std::endl;
			//gameObject->m_xmf4x4ToParent._41 -= 0.005f;
			dwDirection |= DIR_LEFT;
			gameObject->SetAnimationSet(3);
		}
		if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
		{
			std::cout << "d키" << std::endl;
			//gameObject->m_xmf4x4ToParent._41 += 0.005f;
			dwDirection |= DIR_RIGHT;
			gameObject->SetAnimationSet(4);
		}
		if (keyBuffer[VK_SPACE] & 0xF0)
		{
			std::cout << "스페이스바" << std::endl;
			gameObject->SetAnimationSet(5);
		}
		else {
			//gameObject->SetAnimationSet(0);
		}
		if (dwDirection)
		{
			Input::GetInstance()->m_pPlayer->Move(dwDirection, 100.f, true);
			//std::cout << Input::GetInstance()->m_pPlayer->GetPosition().x << "  "
			//	<< Input::GetInstance()->m_pPlayer->GetPosition().y << "  "
			//	<< Input::GetInstance()->m_pPlayer->GetPosition().z << "  "
			//	<< std::endl;

			//std::cout << Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition().x << "  "
			//	<< Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition().y << "  "
			//	<< Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition().z << "  "
			//	<< std::endl;
			//Input::GetInstance()->m_pPlayer->update(elapsedTime);
		}
}
