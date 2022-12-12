#pragma once
#pragma warning(disable:4996)

#include <iostream>
#include <fstream>
#include <winsock2.h>

//#pragma comment(lib, "winnm.lib")
#pragma comment(lib, "ws2_32")

class Voice_Chat {
private:
	WAVEFORMATEX	m_format;    // 음성 포멧

	WAVEHDR			m_waveInHdr; // 입력 데이터

	HWAVEIN			m_hWaveIn;	 // 음성 입력 장치 (마이크)구조체

	WAVEHDR			m_waveOutHdr;// 음성 출력 데이터 구조체

	HWAVEOUT		m_hWaveOut;	 // 음성 출력 장치 (스피커)구조체

	SOCKET			m_socket;	 // 음성 전달 소켓



public:
	Voice_Chat();
	~Voice_Chat();

	void Init();
	void Receive();
	void Listen(int);
	void CALLBACK waveInProc(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void Connect(const char*, int);
};