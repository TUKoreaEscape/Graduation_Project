#pragma once
#include "UI.h"

RECT_FLOAT idRect{ 0,0,0,0 };
RECT_FLOAT passwordRect{ 0,0,0,0 };
RECT_FLOAT roominfoRect[6]{ 0, };

void UpdateRectSize(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	float clientWidth = clientRect.right - clientRect.left;
	float clientHeight = clientRect.bottom - clientRect.top;

	idRect = RECT_FLOAT{ clientWidth / 9.14, clientHeight / 1.62, clientWidth / 2.73, clientHeight / 1.51 };
	passwordRect = RECT_FLOAT{ clientWidth / 9.14, clientHeight / 1.46, clientWidth / 2.73, clientHeight / 1.37 };

	roominfoRect[0] = RECT_FLOAT{ clientWidth / 5.71, clientHeight / 20.0, clientWidth / 2.35, clientHeight / 4.00 };
	roominfoRect[1] = RECT_FLOAT{ clientWidth / 1.73, clientHeight / 20.0, clientWidth / 1.21, clientHeight / 4.00 };
	roominfoRect[2] = RECT_FLOAT{ clientWidth / 5.71, clientHeight / 2.85, clientWidth / 2.35, clientHeight / 1.81 };
	roominfoRect[3] = RECT_FLOAT{ clientWidth / 1.73, clientHeight / 2.85, clientWidth / 1.21, clientHeight / 1.81 };
	roominfoRect[4] = RECT_FLOAT{ clientWidth / 5.71, clientHeight / 1.53, clientWidth / 2.35, clientHeight / 1.17 };
	roominfoRect[5] = RECT_FLOAT{ clientWidth / 1.73, clientHeight / 1.53, clientWidth / 1.21, clientHeight / 1.17 };
}