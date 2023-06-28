#pragma once
#include "UI.h"

RECT_FLOAT idRect{ 0,0,0,0 };
RECT_FLOAT passwordRect{ 0,0,0,0 };
RECT_FLOAT roominfoRect[6]{ 0, };
RECT_FLOAT roomPageRect[2]{ 0, };
RECT_FLOAT pageNumRect{ 0,0,0,0 };
RECT_FLOAT waitingRoomRect[3]{ 0, };
RECT_FLOAT customizingRect{ 0,0,0,0 };
RECT_FLOAT endingRect{ 0,0,0,0 };

void UpdateRectSize(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	float clientWidth = clientRect.right - clientRect.left;
	float clientHeight = clientRect.bottom - clientRect.top;

	idRect = RECT_FLOAT{ clientWidth / float(1600.0 / 175.0), clientHeight / float(900.0 / 555.0), clientWidth / float(1600.0 / 585.0), clientHeight / float(900.0 / 595.0) };
	passwordRect = RECT_FLOAT{ clientWidth / float(1600.0 / 175.0) , clientHeight / float(900.0 / 615.0), clientWidth / float(1600.0 / 585.0), clientHeight / float(900.0 / 656.0) };

	roominfoRect[0] = RECT_FLOAT{ clientWidth / float(1600.0 / 280.0), clientHeight / float(900.0 / 45.0), clientWidth / float(1600.0 / 680.0), clientHeight / float(900.0 / 225.0) };
	roominfoRect[1] = RECT_FLOAT{ clientWidth / float(1600.0 / 920.0), clientHeight / float(900.0 / 45.0), clientWidth / float(1600.0 / 1320.0), clientHeight / float(900.0 / 225.0) };
	roominfoRect[2] = RECT_FLOAT{ clientWidth / float(1600.0 / 280.0), clientHeight / float(900.0 / 315.0), clientWidth / float(1600.0 / 680.0), clientHeight / float(900.0 / 495.0) };
	roominfoRect[3] = RECT_FLOAT{ clientWidth / float(1600.0 / 920.0), clientHeight / float(900.0 / 315.0), clientWidth / float(1600.0 / 1320.0), clientHeight / float(900.0 / 495.0) };
	roominfoRect[4] = RECT_FLOAT{ clientWidth / float(1600.0 / 280.0), clientHeight / float(900.0 / 585.0), clientWidth / float(1600.0 / 680.0), clientHeight / float(900.0 / 765.0) };
	roominfoRect[5] = RECT_FLOAT{ clientWidth / float(1600.0 / 920.0), clientHeight / float(900.0 / 585.0), clientWidth / float(1600.0 / 1320.0), clientHeight / float(900.0 / 765.0) };

	roomPageRect[0] = RECT_FLOAT{ clientWidth / float(1600.0 / 560.0), clientHeight / float(900.0 / 825.0), clientWidth / float(1600.0 / 720.0), clientHeight / float(900.0 / 885.0) };
	roomPageRect[1] = RECT_FLOAT{ clientWidth / float(1600.0 / 880.0), clientHeight / float(900.0 / 825.0), clientWidth / float(1600.0 / 1035.0), clientHeight / float(900.0 / 885.0) };

	pageNumRect = RECT_FLOAT{ clientWidth / float(1600.0 / 780.0), clientHeight / float(900.0 / 850.0), clientWidth / float(1600.0 / 820.0), clientHeight / float(900.0 / 850.0) };

	waitingRoomRect[0] = RECT_FLOAT{ clientWidth / float(1600.0 / 1060.0), clientHeight / float(900.0 / 785.0), clientWidth / float(1600.0 / 1180.0), clientHeight / float(900.0 / 830.0) };
	waitingRoomRect[1] = RECT_FLOAT{ clientWidth / float(1600.0 / 1220.0), clientHeight / float(900.0 / 785.0), clientWidth / float(1600.0 / 1340.0), clientHeight / float(900.0 / 830.0) };
	waitingRoomRect[2] = RECT_FLOAT{ clientWidth / float(1600.0 / 1380.0), clientHeight / float(900.0 / 785.0), clientWidth / float(1600.0 / 1500.0), clientHeight / float(900.0 / 830.0) };
	
	customizingRect = RECT_FLOAT{ clientWidth / float(1600.0 / 1380.0), clientHeight / float(900.0 / 785.0), clientWidth / float(1600.0 / 1500.0), clientHeight / float(900.0 / 830.0) };

	endingRect = RECT_FLOAT{ clientWidth / float(1600.0 / 1380.0), clientHeight / float(900.0 / 785.0), clientWidth / float(1600.0 / 1500.0), clientHeight / float(900.0 / 830.0) };
}