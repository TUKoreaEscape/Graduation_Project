#pragma once
#include "stdafx.h"
#include "Time.h"
#include "Player.h"

class Input
{
private:
	static Input*		InputInstance;
	Input() {}
	Input(const Input& other);
	~Input() {}
public:
	static Input* GetInstance() {
		if (InputInstance == NULL) {
			InputInstance = new Input;
		}
		return InputInstance;
	}

	static UCHAR keyBuffer[256];

	Player*			m_pPlayer=NULL;
	Time				m_time;
	POINT				m_ptOldCursorPos{ 0,0 };

	void Update(HWND hWnd);
	void KeyBoard(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void Mouse(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

//input 클래스는 입력을 처리하기 위한 클래스입니다.
// update 함수에서는 지금의 keystate를 업데이트 해주는 역할을 합니다.
// keyboard 함수에서는 모든 플레이어가 기본적으로 가지게 될 esc키와 특수 키들을 정의할 것입니다.
// mouse 함수에서는 기본적인 마우스 움직임만 관리하고 마우스 클릭에 따른 상호작용은 Movement에서 관리합니다.