#include "stdafx.h"
#include "Movement.h"
#include "Input.h"

void CommonMovement::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CommonMovement::update(float elapsedTime)
{
		UCHAR keyBuffer[256];
		DWORD dwDirection = 0;
		bool		m_emptyKey = false;
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
		if (keyBuffer[VK_SPACE] & 0xF0)
		{
			//std::cout << "스페이스바" << std::endl;
			dwDirection |= DIR_UP;
			gameObject->SetTrackAnimationSet(0, 5);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_UP);
		}
		if (dwDirection)
		{
			if ((dwDirection == DIR_NOT_FB) ||
				(dwDirection == DIR_NOT_LR) ||
				(dwDirection == DIR_NOT_FBLR))
				m_emptyKey = true;
			//std::cout << elapsedTime << std::endl;
			Input::GetInstance()->m_pPlayer->Move(dwDirection, 125.f * elapsedTime, true);
		}
		else m_emptyKey = true;

		if (m_emptyKey)
		{
			gameObject->SetTrackAnimationSet(0, 0);
			Input::GetInstance()->m_pPlayer->SetDirection(DIR_NOT_FB);
		}
}
