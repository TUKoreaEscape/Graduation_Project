#pragma once
#include "Voice_chat.h"

void CALLBACK waveInProc(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    switch (uMsg)
    {
        // �����Ͱ� ȣ��Ǹ� �޽����� WIM_DATA�� ȣ��ȴ�.
    case WIM_DATA:
    {
        // ���� ������ ����ü ���
        WAVEHDR* WaveInHdr = (WAVEHDR*)dwParam1;
        // ������ �����´�.
        SOCKET pClient = (SOCKET)WaveInHdr->dwUser;
        // ���� ������ ����
        send(pClient, WaveInHdr->lpData, WaveInHdr->dwBytesRecorded, 0);
        // ��ġ�� ���� �غ� �˸��� �Լ�
        if (waveInPrepareHeader(hWaveIn, WaveInHdr, sizeof(WAVEHDR))) {
            // ���� �ܼ� ���
            std::cout << "waveInPrepareHeader error" << std::endl;
            // ��ġ �ݱ�
            waveInClose(hWaveIn);
            return;
        }
        // ��ġ�� �޸𸮸� �Ҵ��� �ϴ� �Լ�
        if (waveInAddBuffer(hWaveIn, WaveInHdr, sizeof(WAVEHDR))) {
            // ���� �ܼ� ���
            std::cout << "waveInPrepareHeader error" << std::endl;
            // ��ġ �ݱ�
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

    // ���� ����
    m_format.wFormatTag = WAVE_FORMAT_PCM;
    // ä�� ����
    m_format.nChannels = 1;
    // ���ø� ������ Ƚ��.
    // nSamplesPerSec���� 8.0 kHz, 11.025 kHz, 22.05 kHz, 44.1 kHz�� �ֽ��ϴ�.
    m_format.nSamplesPerSec = 8000;
    // 1ȸ ���ø��� ���Ǿ����� ������ ��Ʈ��, 8 �Ǵ� 16
    m_format.wBitsPerSample = 8;
    // ���ø��� ���Ǵ� ����Ʈ ������ �޸� ũ��
    m_format.nBlockAlign = m_format.nChannels * (m_format.wBitsPerSample / 8);
    // WAVE_FORMAT_PCM������ ���õǴ� ��  
    m_format.cbSize = 0;
    // WAVE_FORMAT_PCM�̶�� �������̱� ������ nSamplesPerSec�� ���� ���̴�.    
    m_format.nAvgBytesPerSec = m_format.nSamplesPerSec;
    // ���̺귯������ ���� ������ ����� ���ϴ� �Լ�(����ڰ� ����ϴ� ���� �ƴ�)    
    m_waveInHdr.dwBytesRecorded = 0;
    m_waveOutHdr.dwBytesRecorded = 0;
    // ���̺귯������ callback �Լ� ���� ���Ǵ� status flag    
    m_waveInHdr.dwFlags = 0;
    // �ݺ� ����� ���� (������� �ʴ´�.)    
    m_waveInHdr.dwLoops = 0;
    // ���� ����� ���� (������� �ʴ´�.)    
    m_waveInHdr.reserved = 0;
    // ���̺귯������ callback �Լ� ���� ���Ǵ� status flag    
    m_waveOutHdr.dwFlags = 0;
    // �ݺ� ����� ���� (������� �ʴ´�.)    
    m_waveOutHdr.dwLoops = 0;
    // ���� ����� ���� (������� �ʴ´�.)    
    m_waveOutHdr.reserved = 0;
    // ������ ���� ����(�Է� ��ġ)
    m_waveInHdr.dwBufferLength = m_format.nAvgBytesPerSec;
    m_waveInHdr.lpData = (char*)malloc(m_waveInHdr.dwBufferLength);
    // ������ ���� ����(��� ��ġ)
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
    // m_waveInHdr.dwUser�� ���� ������ �ִ´�.
    m_waveInHdr.dwUser = (DWORD_PTR)m_socket;
    // �Է� ��ġ ����
    if (waveInOpen(&m_hWaveIn, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION)) {
        // ���� �ܼ� ���
        std::cout << "Failed to open waveform input device." << std::endl;
        throw - 1;
    }
    // ��ġ�� ���� �غ� �˸��� �Լ�
    if (waveInPrepareHeader(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR))) {
        // ���� �ܼ� ���
        std::cout << "waveInPrepareHeader error" << std::endl;
        throw - 1;
    }
    // ���� ����
    if (waveInAddBuffer(m_hWaveIn, &m_waveInHdr, sizeof(WAVEHDR))) {
        // ���� �ܼ� ���
        std::cout << "waveInAddBuffer error" << std::endl;
        throw - 1;
    }
    // ��� ��ġ ����
    if (waveOutOpen(&m_hWaveOut, waveInGetNumDevs() - 1 /*WAVE_MAPPER*/, &m_format, 0, 0, WAVE_FORMAT_DIRECT)) {
        // ���� �ܼ� ���    
        std::cout << "Failed to open waveform output device." << std::endl;
        throw;
    }

}

void Voice_Chat::Receive()
{
    // �ش�κ��� Ŭ���̾�Ʈ���� ������ ����!
    // ���ѷ��� recv�� �ƴ� packet�� ������ ���� wav������ ��ȯ �� �ʿ� ����!

    while (true) {
        // ������ �ޱ�
        recv(m_socket, m_waveOutHdr.lpData, m_waveOutHdr.dwBufferLength, 0);
        // ��� ��ġ �缳��
        if (waveOutReset(m_hWaveOut)) {
            // ���� �ܼ� ���    
            std::cout << "waveOutReset error" << std::endl;
            // ��ġ �ݱ�    
            throw - 1;
        }
        // ��ġ�� ��� �غ� �˸��� �Լ�    
        if (waveOutPrepareHeader(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR))) {
            // ���� �ܼ� ���    
            std::cout << "waveOutPrepareHeader error" << std::endl;
            // ��ġ �ݱ�    
            throw - 1;
        }
        // ��� ����
        if (waveOutWrite(m_hWaveOut, &m_waveOutHdr, sizeof(WAVEHDR))) {
            // ���� �ܼ� ���
            std::cout << "waveOutWrite error" << std::endl;
            // ��ġ �ݱ�
            throw - 1;
        }
    }
}