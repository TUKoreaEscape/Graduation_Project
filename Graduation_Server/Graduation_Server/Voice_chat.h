#pragma once
#pragma warning(disable:4996)

#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")
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
};