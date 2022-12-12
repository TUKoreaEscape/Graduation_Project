#pragma once
#pragma warning(disable:4996)

#include <iostream>
#include <fstream>
#include <winsock2.h>

//#pragma comment(lib, "winnm.lib")
#pragma comment(lib, "ws2_32")

class Voice_Chat {
private:
	WAVEFORMATEX	m_format;    // ���� ����

	WAVEHDR			m_waveInHdr; // �Է� ������

	HWAVEIN			m_hWaveIn;	 // ���� �Է� ��ġ (����ũ)����ü

	WAVEHDR			m_waveOutHdr;// ���� ��� ������ ����ü

	HWAVEOUT		m_hWaveOut;	 // ���� ��� ��ġ (����Ŀ)����ü

	SOCKET			m_socket;	 // ���� ���� ����



public:
	Voice_Chat();
	~Voice_Chat();

	void Init();
	void Receive();
	void Listen(int);
	void CALLBACK waveInProc(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void Connect(const char*, int);
};