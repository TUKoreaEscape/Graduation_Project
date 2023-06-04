#pragma once
#include "stdafx.h"
#include "Framework.h"
#include "Input.h"
#include "Network.h"
#include "Graduation_Client.h"

Input* Input::InputInstance = nullptr;
UCHAR Input::keyBuffer[256];

void Input::Update(HWND hWnd)
{
	GetKeyboardState(keyBuffer);

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	if (GetCapture() == hWnd)
	{
		SetCursor(NULL);
		GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 5.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 5.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if (cxDelta || cyDelta)
	{
		if (m_pPlayer) {
			if (keyBuffer[VK_RBUTTON] & 0xF0) m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
			else m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
		}
	}
}

void Input::KeyBoard(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	POINT ptCenter = { (rcClient.right - rcClient.left) / 2, (rcClient.bottom - rcClient.top) / 2 };
	POINT ptMouse;
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
			GetCursorPos(&ptMouse);
			if (ptMouse.x != ptCenter.x || ptMouse.y != ptCenter.y) {
				SetCursorPos(ptCenter.x, ptCenter.y);
			}
			::SetCapture(hWnd);
			::GetCursorPos(&m_ptOldCursorPos);
			break;
		case VK_F2:
			::ReleaseCapture();
			break;
		case VK_F4:
			speed = 160.0f;
			break;
		case VK_F5:
			speed = 60.0f;
			break;
		case VK_F6:
			m_gamestate->ChangeState();
			break;
		case VK_ESCAPE:
		{
			//::PostQuitMessage(0);
			break;
		}
		case VK_RETURN:
		{
#if USE_NETWORK
			if (m_gamestate->GetGameState() == LOGIN)
			{
				Network& network = *Network::GetInstance();
				m_cs_packet_login.size = sizeof(m_cs_packet_login);
				m_cs_packet_login.type = CS_PACKET::CS_PACKET_LOGIN;

				network.send_packet(&m_cs_packet_login);
			}
#endif
			break;
		}

		case VK_F9:
			g_Framework.ChangeSwapChainState();
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		if (m_gamestate->GetGameState() == LOGIN)
		{
			if (wParam == VK_BACK) {
				if (m_inputState == 1) DeleteIdAndPassword(m_cs_packet_login.id, m_idNum);//id 삭제
				else if (m_inputState == 2) DeleteIdAndPassword(m_cs_packet_login.pass_word, m_passwordNum); //password 삭제
			}
			else if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9) {
				char number = wParam - VK_NUMPAD0 + '0';
				if (m_inputState == 1) InputIdAndPassword(number, m_cs_packet_login.id, m_idNum);//id입력
				else if (m_inputState == 2) InputIdAndPassword(number, m_cs_packet_login.pass_word, m_passwordNum); //password입력
			}
			else if (wParam == VK_TAB) { ChangeInputState(); }
			else if ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z') || (wParam >= '0' && wParam <= '9'))
			{
				if (m_inputState == 1) InputIdAndPassword(wParam, m_cs_packet_login.id, m_idNum);//id입력
				else if (m_inputState == 2) InputIdAndPassword(wParam, m_cs_packet_login.pass_word, m_passwordNum); //password입력
			}
		}
		break;
	default:
		break;
	}
}

void Input::Mouse(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (!m_pPlayer->IsAttack() && m_gamestate->GetGameState() == PLAYING_GAME)
		{
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			network.Send_Attack_Packet();
			m_pPlayer->SetAttackZeroTime();
#endif
		}
		if (m_gamestate->GetGameState() == LOGIN)
		{
			UpdateRectSize(hWnd);
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			//std::cout << idInputRect.left << " " << idInputRect.top << " " << idInputRect.right << " " << idInputRect.bottom << std::endl;
			//std::cout << passwordInputRect.left << " " << passwordInputRect.top << " " << passwordInputRect.right << " " << passwordInputRect.bottom << std::endl;
			//std::cout << xPos << " " << yPos << std::endl;
			//아이디
			if (xPos >= idRect.left && xPos <= idRect.right && yPos >= idRect.top && yPos <= idRect.bottom)
			{
				// 입력란을 활성화하고 입력을 받을 수 있는 상태로 전환합니다.
				m_inputState = 1;
				std::cout << "아이디 칸 클릭!" << std::endl;
			}
			//비밀번호
			else if (xPos >= passwordRect.left && xPos <= passwordRect.right && yPos >= passwordRect.top && yPos <= passwordRect.bottom)
			{
				m_inputState = 2;
				std::cout << "비밀번호 칸 클릭!" << std::endl;
			}
			else
			{
				m_inputState = 0;
				std::cout << "빈칸 클릭!" << std::endl;
			}
		}
		else if (m_gamestate->GetGameState() == ROOM_SELECT)
		{
			UpdateRectSize(hWnd);
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			for (int i = 0; i < 6; ++i)
			{
				if (xPos >= roominfoRect[i].left && xPos <= roominfoRect[i].right && yPos >= roominfoRect[i].top && yPos <= roominfoRect[i].bottom) //어떤 방을 클릭했는지 판단하는 코드
				{
					std::cout << i+1  << " 방 클릭!" << std::endl;
					Network& network = *Network::GetInstance();
					network.Send_Select_Room(i);
				}
			}
			//InputRoomInfo();
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

LRESULT Input::ProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_time.Stop();
		else
			m_time.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		Mouse(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		KeyBoard(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void Input::InputIdAndPassword(char input_char, char* str, int& num)
{
	if (isalpha(char(input_char)))
	{
		if (GetKeyState(VK_SHIFT) < 0) // Shift 키가 눌린 상태인지 확인
		{
			//std::cout << input_char << std::endl;
			//std::cout << num << std::endl;
			if (num < MAX_NAME_SIZE - 1)
			{
				str[num] = char(input_char);
				num++;
			}
		}
		else
		{
			//std::cout << char(tolower(input_char)) << std::endl;
			if (num < MAX_NAME_SIZE - 1)
			{
				str[num] = char(tolower(input_char));
				num++;
			}
		}
	}
	else
	{
		if (num < MAX_NAME_SIZE - 1)
		{
			str[num] = char(input_char);
			num++;
		}
	}
	//std::cout << str << std::endl;
}

void Input::DeleteIdAndPassword(char* str, int& num)
{
	if (num > 0)
	{
		str[num] = '\0';
		num--;
	}
	else if (num == 0)
	{
		str[num] = '\0';
	}
	//std::cout << str << std::endl;
}

void Input::InputRoomInfo()
{
	m_Roominfo[0].room_number = 1;
	strcpy_s(m_Roominfo[0].room_name, "come~\0");
	m_Roominfo[0].join_member = 1;
	m_Roominfo[0].state = GAME_ROOM_STATE::FREE;

	m_Roominfo[1].room_number = 2;
	strcpy_s(m_Roominfo[1].room_name, "run!\0");
	m_Roominfo[1].join_member = 2;
	m_Roominfo[1].state = GAME_ROOM_STATE::READY;

	m_Roominfo[2].room_number = 3;
	strcpy_s(m_Roominfo[2].room_name, "gogo\0");
	m_Roominfo[2].join_member = 6;
	m_Roominfo[2].state = GAME_ROOM_STATE::PLAYING;

	m_Roominfo[3].room_number = 4;
	strcpy_s(m_Roominfo[3].room_name, "try\0");
	m_Roominfo[3].join_member = 5;
	m_Roominfo[3].state = GAME_ROOM_STATE::READY;

	m_Roominfo[4].room_number = 5;
	strcpy_s(m_Roominfo[4].room_name, "fail\0");
	m_Roominfo[4].join_member = 2;
	m_Roominfo[4].state = GAME_ROOM_STATE::FREE;

	m_Roominfo[5].room_number = 6;
	strcpy_s(m_Roominfo[5].room_name, "plz\0");
	m_Roominfo[5].join_member = 1;
	m_Roominfo[5].state = GAME_ROOM_STATE::END;
}