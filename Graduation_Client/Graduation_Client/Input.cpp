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
			m_gamestate->ChangeNextState();
			break;
		case VK_F7:
			m_gamestate->ChangePrevState();
			break;
		case VK_F8:
			m_gamestate->ChangeSameLevelState();
			break;
		case VK_F3:
			if (m_pPlayer) {
				if (m_pPlayer->m_Type == TYPE_TAGGER) m_pPlayer->SetPlayerType(TYPE_PLAYER);
				else m_pPlayer->SetPlayerType(TYPE_TAGGER);
			}
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
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);
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
			}
			//비밀번호
			else if (xPos >= passwordRect.left && xPos <= passwordRect.right && yPos >= passwordRect.top && yPos <= passwordRect.bottom)
			{
				m_inputState = 2;
			}
			else if (xPos >= logininfoRect[0].left && xPos <= logininfoRect[0].right && yPos >= logininfoRect[0].top && yPos <= logininfoRect[0].bottom) //Login
			{
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				m_cs_packet_login.size = sizeof(m_cs_packet_login);
				m_cs_packet_login.type = CS_PACKET::CS_PACKET_LOGIN;

				network.send_packet(&m_cs_packet_login);
#endif
			}
			else if (xPos >= logininfoRect[1].left && xPos <= logininfoRect[1].right && yPos >= logininfoRect[1].top && yPos <= logininfoRect[1].bottom) //Create ID
			{
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				cs_packet_create_id packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_CREATE_ID;
				memcpy(packet.id, m_cs_packet_login.id, sizeof(m_cs_packet_login.id));
				memcpy(packet.pass_word, m_cs_packet_login.pass_word, sizeof(m_cs_packet_login.pass_word));
				
				network.send_packet(&packet);
#endif
			}
			else if (xPos >= logininfoRect[2].left && xPos <= logininfoRect[2].right && yPos >= logininfoRect[2].top && yPos <= logininfoRect[2].bottom)//Login fail
			{
				m_errorState = 0;
				m_SuccessState = 0;
			}
			else if (xPos >= logininfoRect[3].left && xPos <= logininfoRect[3].right && yPos >= logininfoRect[3].top && yPos <= logininfoRect[3].bottom)//Same ID
			{
				m_errorState = 0;
				m_SuccessState = 0;
			}
			else if (xPos >= logininfoRect[4].left && xPos <= logininfoRect[4].right && yPos >= logininfoRect[4].top && yPos <= logininfoRect[4].bottom)//SuccessfullycreatedID
			{
				m_SuccessState = 0;
				m_errorState = 0;
			}
			else
			{
				m_inputState = 0;
				m_SuccessState = 0;
				m_errorState = 0;
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
#if USE_NETWORK
					Network& network = *Network::GetInstance();
					network.Send_Select_Room(6 * (m_PageNum - 1) + i, i);
					network.m_join_room_number = 6 * (m_PageNum - 1) + i;
					network.m_page_num = m_PageNum - 1;
#endif
				}
			}
			for (int i = 0; i < 2; ++i)
			{
				if (xPos >= roomPageRect[i].left && xPos <= roomPageRect[i].right && yPos >= roomPageRect[i].top && yPos <= roomPageRect[i].bottom)
				{
					//페이지 업 다운 판단, 0일때 왼쪽 화살표,1일때 오른쪽 화살표
					// input에 PageNum이 있음 김우빈
					if (i == 0) {
						PageDown();
#if USE_NETWORK
						Network& network = *Network::GetInstance();
						network.Send_Request_Room_Info(m_PageNum - 1);
#endif
					}
					else {
						PageUp();
#if USE_NETWORK
						Network& network = *Network::GetInstance();
						network.Send_Request_Room_Info(m_PageNum - 1);
#endif
					}
				}
			}
			//InputRoomInfo();
		}
		else if (m_gamestate->GetGameState() == WAITING_GAME)
		{
			UpdateRectSize(hWnd);
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			for (int i = 0; i < 3; ++i)
			{
				if (xPos >= waitingRoomRect[i].left && xPos <= waitingRoomRect[i].right && yPos >= waitingRoomRect[i].top && yPos <= waitingRoomRect[i].bottom) //어떤 방을 클릭했는지 판단하는 코드
				{
					//std::cout << i+1  << " 방 클릭!" << std::endl;
					if (i == 0)
					{
#if !USE_NETWORK
						m_gamestate->ChangeNextState();//READY클릭 김우빈 여기수정
#endif
#if USE_NETWORK
						if (m_cs_packet_ready.ready_type == false) {
							Network& network = *Network::GetInstance();
							network.Send_Ready_Packet(true);
							m_cs_packet_ready.ready_type = true;
						}
						else {
							Network& network = *Network::GetInstance();
							network.Send_Ready_Packet(false);
							m_cs_packet_ready.ready_type = false;
						}
#endif
					}
					else if (i == 1)
					{
#if USE_NETWORK
						Network& network = *Network::GetInstance();
						network.Send_Exit_Room();
#endif
						m_gamestate->ChangePrevState();//QUIT클릭
						m_cs_packet_ready.ready_type = false;
					}
					else if (i == 2)
					{
						m_gamestate->ChangeSameLevelState();//CUSTOMIZING클릭
						for (int i = 0; i < 6; ++i) {
							m_nPrevCosInfoIndex[i] = m_pPlayer->FindPlayerPart(i);
						}
#if USE_NETWORK
						Network& network = *Network::GetInstance();
						network.Send_Ready_Packet(false);
#endif
						m_cs_packet_ready.ready_type = false;
					}
				}
			}
			//InputRoomInfo();
		}
		else if (m_gamestate->GetGameState() == CUSTOMIZING)
		{
			UpdateRectSize(hWnd);
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			for(int i=0; i<10; ++i)
			{
				if (xPos >= customizingRect[i].left && xPos <= customizingRect[i].right && yPos >= customizingRect[i].top && yPos <= customizingRect[i].bottom)
				{
					if (i == 0) {
						int partIndex[6]; // 여기에 순서대로 저장됨
						for (int i = 0; i < 6; ++i) {
							m_nPrevCosInfoIndex[i] = partIndex[i] = m_pPlayer->FindPlayerPart(i);
						}
#if USE_NETWORK
						cs_packet_customizing_update packet;
						packet.size = sizeof(packet);
						packet.type = CS_PACKET::CS_PACKET_CUSTOMIZING;
						packet.body = m_nPrevCosInfoIndex[0];
						packet.body_parts = m_nPrevCosInfoIndex[1];
						packet.eyes = m_nPrevCosInfoIndex[2];
						packet.gloves = m_nPrevCosInfoIndex[3];
						packet.head = m_nPrevCosInfoIndex[5];
						packet.mouthandnoses = m_nPrevCosInfoIndex[4];

						Network& network = *Network::GetInstance();
						network.send_packet(&packet);
#endif
					}
					else if (i == 1) {
						m_gamestate->ChangeSameLevelState();//QUIT클릭
						for (int i = 0; i < 6; ++i) {
							m_pPlayer->SetParts(PLAYER, i, m_nPrevCosInfoIndex[i]);
						}
					}
					else if (i == 2) {
						std::cout << "HEAD" << std::endl; // HEAD
						m_nCosIndex = 5;
					}
					else if (i == 3) {
						std::cout << "Eyes" << std::endl; // Eyes
						m_nCosIndex = 2;
					}
					else if (i == 4) {
						std::cout << "Mouthandnoses" << std::endl; //Mouthandnoses
						m_nCosIndex = 4;
					}
					else if (i == 5) {
						std::cout << "Body" << std::endl; // Body
						m_nCosIndex = 0;
					}
					else if (i == 6) {
						std::cout << "BodyParts" << std::endl; //BodyParts
						m_nCosIndex = 1;
					}
					else if (i == 7) {
						std::cout << "Gloves" << std::endl; //Gloves
						m_nCosIndex = 3;
					}
					else if (i == 8) {
						std::cout << "CustomizingRArrow" << std::endl; //CustomizingRArrow
						int n = m_pPlayer->FindPlayerPart(m_nCosIndex);
						m_pPlayer->ChangePlayerPart(m_nCosIndex, n, true);
					}
					else if (i == 9) {
						std::cout << "CustomizingLArrow" << std::endl; //CustomizingLArrow
						int n = m_pPlayer->FindPlayerPart(m_nCosIndex);
						m_pPlayer->ChangePlayerPart(m_nCosIndex, n, false);
					}
				}
			}
			//InputRoomInfo();
		}
		else if (m_gamestate->GetGameState() == ENDING_GAME)
		{
			UpdateRectSize(hWnd);
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			if (xPos >= endingRect.left && xPos <= endingRect.right && yPos >= endingRect.top && yPos <= endingRect.bottom)
			{
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
				network.m_pPlayer->SetTrackAnimationSet(0, 0);

				for (int i = 0; i < 5; ++i) {
					network.m_ppOther[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
					network.m_ppOther[i]->SetTrackAnimationSet(0, 0);
				}

				m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
				for (int i = 0; i < 5; ++i)
					network.m_ppOther[i]->SetPlayerType(TYPE_PLAYER_YET);
#endif
				m_cs_packet_ready.ready_type = false;
				m_gamestate->ChangeNextState();//QUIT클릭
			}
			//InputRoomInfo();
		}
		//std::cout << xPos << " " << yPos << std::endl;
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