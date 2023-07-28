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
	
	//��Ŷ
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

	int								m_inputState = 0; //0->x , 1->Id�Է�, 2->password�Է�
	int								m_errorState = 0;
	int								m_SuccessState = 0;
	bool							m_debuglight = false;
	bool							m_curstateon = false;
	bool							m_ClickState = false;

	TCHAR tmp[100]{NULL}; //�Է¹޴� ���ڿ��� ���� ����
	bool bComposite; //�Է� ���̶� ���� ���� ������ �ƴ��� �Ǻ��ϱ� ���� ����

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
	void InputRoomInfo(); //�� �Լ����� roominfo�� �־��� ��Ŷ�ְ�޴°��� ������ �ɵ�? �ȿ� �������ְ�, �ϴ� ������ input.cpp 159���ٿ� �־����
	void Receive(char* chat, char* name); //Receive ä�ð� �̸��� �����ָ� ��
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

//input Ŭ������ �Է��� ó���ϱ� ���� Ŭ�����Դϴ�.
// update �Լ������� ������ keystate�� ������Ʈ ���ִ� ������ �մϴ�.
// keyboard �Լ������� ��� �÷��̾ �⺻������ ������ �� escŰ�� Ư�� Ű���� ������ ���Դϴ�.
// mouse �Լ������� �⺻���� ���콺 �����Ӹ� �����ϰ� ���콺 Ŭ���� ���� ��ȣ�ۿ��� Movement���� �����մϴ�.