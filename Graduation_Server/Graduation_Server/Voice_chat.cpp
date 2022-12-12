#pragma once
#include "Voice_chat.h"

void CALLBACK waveInProc(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    switch (uMsg)
    {
        // 데이터가 호출되면 메시지가 WIM_DATA로 호출된다.
    case WIM_DATA:
    {
        // 음성 데이터 구조체 취득
        WAVEHDR* WaveInHdr = (WAVEHDR*)dwParam1;
        // 소켓을 가져온다.
        SOCKET pClient = (SOCKET)WaveInHdr->dwUser;
        // 음성 데이터 전송
        send(pClient, WaveInHdr->lpData, WaveInHdr->dwBytesRecorded, 0);
        // 장치에 녹음 준비를 알리는 함수
        if (waveInPrepareHeader(hWaveIn, WaveInHdr, sizeof(WAVEHDR))) {
            // 에러 콘솔 출력
            std::cout << "waveInPrepareHeader error" << std::endl;
            // 장치 닫기
            waveInClose(hWaveIn);
            return;
        }
        // 장치에 메모리를 할당을 하는 함수
        if (waveInAddBuffer(hWaveIn, WaveInHdr, sizeof(WAVEHDR))) {
            // 에러 콘솔 출력
            std::cout << "waveInPrepareHeader error" << std::endl;
            // 장치 닫기
            waveInClose(hWaveIn);
            return;
        }
    }
    break;
    }
}


Voice_Chat::Voice_Chat()
{
    memset(&m_format, 0x00, sizeof(WAVEFORMATEX));
    memset(&m_waveInHdr, 0x00, sizeof(WAVEHDR));
    memset(&m_waveOutHdr, 0x00, sizeof(WAVEHDR));
    memset(&m_hWaveIn, 0x00, sizeof(HWAVEIN));
    memset(&m_hWaveOut, 0x00, sizeof(HWAVEOUT));

    // 포멧 설정
    m_format.wFormatTag = WAVE_FORMAT_PCM;
    // 채널 설정
    m_format.nChannels = 1;
    // 샘플링 데이터 횟수.
    // nSamplesPerSec에는 8.0 kHz, 11.025 kHz, 22.05 kHz, 44.1 kHz가 있습니다.
    m_format.nSamplesPerSec = 8000;
    // 1회 샘플링에 사용되어지는 데이터 비트수, 8 또는 16
    m_format.wBitsPerSample = 8;
    // 샘플링에 사용되는 바이트 단위의 메모리 크기
    m_format.nBlockAlign = m_format.nChannels * (m_format.wBitsPerSample / 8);
    // WAVE_FORMAT_PCM에서는 무시되는 값  
    m_format.cbSize = 0;
    // WAVE_FORMAT_PCM이라면 무압축이기 때문에 nSamplesPerSec와 같을 것이다.    
    m_format.nAvgBytesPerSec = m_format.nSamplesPerSec;
    // 라이브러리에서 실제 녹음된 사이즈를 구하는 함수(사용자가 사용하는 값이 아님)    
    m_waveInHdr.dwBytesRecorded = 0;
    m_waveOutHdr.dwBytesRecorded = 0;
    // 라이브러리에서 callback 함수 사용시 사용되는 status flag    
    m_waveInHdr.dwFlags = 0;
    // 반복 재생시 사용됨 (사용하지 않는다.)    
    m_waveInHdr.dwLoops = 0;
    // 예약 재생시 사용됨 (사용하지 않는다.)    
    m_waveInHdr.reserved = 0;
    // 라이브러리에서 callback 함수 사용시 사용되는 status flag    
    m_waveOutHdr.dwFlags = 0;
    // 반복 재생시 사용됨 (사용하지 않는다.)    
    m_waveOutHdr.dwLoops = 0;
    // 예약 재생시 사용됨 (사용하지 않는다.)    
    m_waveOutHdr.reserved = 0;
    // 데이터 버퍼 설정(입력 장치)
    m_waveInHdr.dwBufferLength = m_format.nAvgBytesPerSec;
    m_waveInHdr.lpData = (char*)malloc(m_waveInHdr.dwBufferLength);
    // 데이터 버퍼 설정(출력 장치)
    m_waveOutHdr.dwBufferLength = m_format.nAvgBytesPerSec;
    m_waveOutHdr.lpData = (char*)malloc(m_waveInHdr.dwBufferLength);
}

Voice_Chat::~Voice_Chat()
{
    waveInClose(m_hWaveIn);
    waveOutClose(m_hWaveOut);
}

void Voice_Chat::Init()
{
    // m_waveInHdr.dwUser에 소켓 정보를 넣는다.
    m_waveInHdr.dwUser = (DWORD_PTR)m_socket;
    // 입력 장치 설정
    if (waveInOpen(&m_hWaveIn, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION)) {
        // 에러 콘솔 출력
        std::cout << "Failed to open waveform input device." << std::endl;
        throw - 1;
    }
    // 장치에 녹음 준비를 알리는 함수
    if (waveInPrepareHeader(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR))) {
        // 에러 콘솔 출력
        std::cout << "waveInPrepareHeader error" << std::endl;
        throw - 1;
    }
    // 버퍼 설정
    if (waveInAddBuffer(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR))) {
        // 에러 콘솔 출력
        std::cout << "waveInAddBuffer error" << std::endl;
        throw - 1;
    }
    // 출력 장치 설정
    if (waveOutOpen(&m_hWaveOut, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, 0, 0, WAVE_FORMAT_DIRECT)) {
        // 에러 콘솔 출력    
        std::cout << "Failed to open waveform output device." << std::endl;
        throw;
    }

}

void Voice_Chat::Receive()
{
    // 해당부분은 클라이언트에서 수정할 예정!
    // 무한루프 recv가 아닌 packet의 정보를 보고 wav파일을 변환 할 필요 있음!

    while (true) {
        // 데이터 받기
        recv(m_socket, m_waveOutHdr.lpData, m_waveOutHdr.dwBufferLength, 0);
        // 출력 장치 재설정
        if (waveOutReset(m_hWaveOut)) {
            // 에러 콘솔 출력    
            std::cout << "waveOutReset error" << std::endl;
            // 장치 닫기    
            throw - 1;
        }
        // 장치에 출력 준비를 알리는 함수    
        if (waveOutPrepareHeader(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR))) {
            // 에러 콘솔 출력    
            std::cout << "waveOutPrepareHeader error" << std::endl;
            // 장치 닫기    
            throw - 1;
        }
        // 출력 시작
        if (waveOutWrite(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR))) {
            // 에러 콘솔 출력
            std::cout << "waveOutWrite error" << std::endl;
            // 장치 닫기
            throw - 1;
        }
    }
}