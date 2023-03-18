#pragma once
#include "Movement.h"
#include "Input.h"

void CommonMovement::start()
{
	m_pPlayer = static_cast<Player*>(this->gameObject);
}

void CommonMovement::update(float elapsedTime)
{
	if (m_pPlayer)
	{
		UCHAR keyBuffer[256];
		memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
		if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		{
			//m_pPlayer를 사용하여 움직이는 코드를 작성하면 된다.
			std::cout << "w키" << std::endl;
		}
		if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		{
			std::cout << "s키" << std::endl;
		}
		if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		{
			std::cout << "a키" << std::endl;
		}
		if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
		{
			std::cout << "d키" << std::endl;
		}
		if (keyBuffer[VK_SPACE] & 0xF0)
		{
			std::cout << "스페이스바" << std::endl;
		}
	}
}
