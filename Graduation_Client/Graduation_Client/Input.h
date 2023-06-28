#pragma once
#include "Time.h"
#include "Player.h"
#include "protocol.h"
#include "Game_state.h"

class Input
{
private:
	static Input*				InputInstance;
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

	static UCHAR		keyBuffer[256];
	Player*			m_pPlayer=NULL;
	Time				m_time;
	POINT				m_ptOldCursorPos{ 0,0 };
	
	//패킷
	cs_packet_login				m_cs_packet_login{ NULL};
	Roominfo_by10				m_Roominfo[6]{ NULL };
	cs_packet_ready			m_cs_packet_ready{ NULL };
	int									m_PageNum = 1;

	int								m_idNum = 0;
	int								m_passwordNum = 0;

	int								m_inputState = 0; //0->x , 1->Id입력, 2->password입력

	float speed = 60.0f;

	GameState*		m_gamestate;

	void Update(HWND hWnd);
	void KeyBoard(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void Mouse(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void InputIdAndPassword(char input_char, char* str, int& num);
	void DeleteIdAndPassword(char* str, int& num);
	int InputState() { return m_inputState; };
	void PageUp() { m_PageNum++; };
	void PageDown() { if (m_PageNum > 1)m_PageNum--; };
	int ChangeInputState() { return m_inputState = (m_inputState + 1) % 3; };
	void InputRoomInfo(); //이 함수에서 roominfo를 넣어줌 패킷주고받는곳에 넣으면 될듯? 안에 정의해주고, 일단 지금은 input.cpp 159번줄에 넣어놨음
};

//input 클래스는 입력을 처리하기 위한 클래스입니다.
// update 함수에서는 지금의 keystate를 업데이트 해주는 역할을 합니다.
// keyboard 함수에서는 모든 플레이어가 기본적으로 가지게 될 esc키와 특수 키들을 정의할 것입니다.
// mouse 함수에서는 기본적인 마우스 움직임만 관리하고 마우스 클릭에 따른 상호작용은 Movement에서 관리합니다.