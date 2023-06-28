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
	
	//��Ŷ
	cs_packet_login				m_cs_packet_login{ NULL};
	Roominfo_by10				m_Roominfo[6]{ NULL };
	cs_packet_ready			m_cs_packet_ready{ NULL };
	int									m_PageNum = 1;

	int								m_idNum = 0;
	int								m_passwordNum = 0;

	int								m_inputState = 0; //0->x , 1->Id�Է�, 2->password�Է�

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
	void InputRoomInfo(); //�� �Լ����� roominfo�� �־��� ��Ŷ�ְ�޴°��� ������ �ɵ�? �ȿ� �������ְ�, �ϴ� ������ input.cpp 159���ٿ� �־����
};

//input Ŭ������ �Է��� ó���ϱ� ���� Ŭ�����Դϴ�.
// update �Լ������� ������ keystate�� ������Ʈ ���ִ� ������ �մϴ�.
// keyboard �Լ������� ��� �÷��̾ �⺻������ ������ �� escŰ�� Ư�� Ű���� ������ ���Դϴ�.
// mouse �Լ������� �⺻���� ���콺 �����Ӹ� �����ϰ� ���콺 Ŭ���� ���� ��ȣ�ۿ��� Movement���� �����մϴ�.