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

	float					m_MouseX = 0;
	float					m_MouseY = 0;

	GameObject* m_pTestDoor = nullptr;
	
	//패킷
	cs_packet_login				m_cs_packet_login{ NULL};
	Roominfo_by10				m_Roominfo[6]{ NULL };
	cs_packet_ready			m_cs_packet_ready{ NULL };
	cs_packet_chat				m_cs_packet_chat{ NULL };
	char								m_chatlist[5][120]{ ""};
	int									m_PageNum = 1;
	int									m_Limitchatlength = 38;

	int								m_idNum = 0;
	int								m_passwordNum = 0;
	int								m_chatNum = 0;

	int								m_inputState = 0; //0->x , 1->Id입력, 2->password입력
	int								m_errorState = 0;
	int								m_SuccessState = 0;
	bool							m_debuglight = false;
	bool							m_curstateon = false;
	bool							m_ClickState = false;

	TCHAR tmp[100]{NULL}; //입력받는 문자열을 담을 버퍼
	bool bComposite; //입력 중이라 문자 조합 중인지 아닌지 판별하기 위한 변수

	float speed = 60.0f;

	GameState*		m_gamestate = GameState::GetInstance();

	int m_nCosIndex{};
	int m_nPrevCosInfoIndex[6];

	void Update(HWND hWnd);
	void KeyBoard(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void Mouse(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void InputIdAndPassword(char input_char, char* str, int& num);
	void Inputchat(WPARAM input_char, char* str, int& num);
	void DeleteIdAndPassword(char* str, int& num);
	int InputState() { return m_inputState; };
	void PageUp() { m_PageNum++; };
	void PageDown() { if (m_PageNum > 1)m_PageNum--; };
	int ChangeInputState() { return m_inputState = (m_inputState + 1) % 3; };
	int ChangeErrorState() { return m_inputState = (m_inputState + 1) % 3; };
	void InputRoomInfo(); //이 함수에서 roominfo를 넣어줌 패킷주고받는곳에 넣으면 될듯? 안에 정의해주고, 일단 지금은 input.cpp 159번줄에 넣어놨음
	void Receive(char* chat, char* name); //Receive 채팅과 이름을 보내주면 됨
	LRESULT OnImeComposition(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT OnImeChar(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool GetcursorState() { return m_curstateon; };
	void SetcursorState() { m_curstateon = !m_curstateon; };
	void CaptureOn(HWND hWnd);
	void CaptureOff();
	bool GetClickState() { return m_ClickState; };
	void SetClickState(bool value) { m_ClickState = value; };
	bool IsWindowCaptured(HWND hWnd, int x, int y);
};

//input 클래스는 입력을 처리하기 위한 클래스입니다.
// update 함수에서는 지금의 keystate를 업데이트 해주는 역할을 합니다.
// keyboard 함수에서는 모든 플레이어가 기본적으로 가지게 될 esc키와 특수 키들을 정의할 것입니다.
// mouse 함수에서는 기본적인 마우스 움직임만 관리하고 마우스 클릭에 따른 상호작용은 Movement에서 관리합니다.