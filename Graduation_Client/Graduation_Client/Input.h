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

//input Ŭ������ �Է��� ó���ϱ� ���� Ŭ�����Դϴ�.
// update �Լ������� ������ keystate�� ������Ʈ ���ִ� ������ �մϴ�.
// keyboard �Լ������� ��� �÷��̾ �⺻������ ������ �� escŰ�� Ư�� Ű���� ������ ���Դϴ�.
// mouse �Լ������� �⺻���� ���콺 �����Ӹ� �����ϰ� ���콺 Ŭ���� ���� ��ȣ�ۿ��� Movement���� �����մϴ�.