#pragma once
#include "stdafx.h"
#include "Input.h"

extern RECT_FLOAT idRect;
extern RECT_FLOAT passwordRect;

extern RECT_FLOAT logininfoRect[4];

extern RECT_FLOAT roominfoRect[6];

extern RECT_FLOAT roomPageRect[2];

extern RECT_FLOAT pageNumRect;

extern RECT_FLOAT waitingRoomRect[3];

extern RECT_FLOAT customizingRect[10];

extern RECT_FLOAT endingRect;

void UpdateRectSize(HWND hWnd);