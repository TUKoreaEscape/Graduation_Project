#include "stdafx.h"
#include "Movement.h"
#include "Input.h"

void CommonMovement::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CommonMovement::update(float elapsedTime)
{
		UCHAR keyBuffer[256];
		memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
		if (keyBuffer['w'] & 0xF0 || keyBuffer['W'] & 0xF0)
		{
			//m_pPlayer를 사용하여 움직이는 코드를 작성하면 된다.
			std::cout << "w키" << std::endl;
			gameObject->m_xmf4x4ToParent._43 += 0.1f;
		}
		if (keyBuffer['s'] & 0xF0 || keyBuffer['S'] & 0xF0)
		{
			std::cout << "s키" << std::endl;
			gameObject->m_xmf4x4ToParent._43 -= 0.1f;
		}
		if (keyBuffer['a'] & 0xF0 || keyBuffer['A'] & 0xF0)
		{
			std::cout << "a키" << std::endl;
			gameObject->m_xmf4x4ToParent._41 -= 0.1f;
		}
		if (keyBuffer['d'] & 0xF0 || keyBuffer['D'] & 0xF0)
		{
			std::cout << "d키" << std::endl;
			gameObject->m_xmf4x4ToParent._41 += 0.1f;
		}
		if (keyBuffer[VK_SPACE] & 0xF0)
		{
			std::cout << "스페이스바" << std::endl;
		}
}
