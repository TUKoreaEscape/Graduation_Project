#pragma once
#include "stdafx.h"
#include "Framework.h"
#include "Graduation_Client.h"
#include "Input.h"
#include "Network.h"
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
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F9:
			g_Framework.ChangeSwapChainState();
		default:
			break;
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
		if (!m_pPlayer->IsAttack())
		{
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			network.Send_Attack_Packet();
			m_pPlayer->SetAttackZeroTime();
#endif
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
