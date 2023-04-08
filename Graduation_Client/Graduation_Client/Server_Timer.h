#pragma once
#include <chrono>
#include <ctime>
#include <Windows.h>

class Server_Timer
{
public:
    Server_Timer()
    {
        reset();
    }

    ~Server_Timer() = default;

    void reset()
    {
        QueryPerformanceFrequency(&m_CountTime);
        QueryPerformanceCounter(&m_CurTime);
        QueryPerformanceCounter(&m_PrevTime);
    }

    float update()
    {
        QueryPerformanceCounter(&m_CurTime);
        m_dDeltaTime = (static_cast<double>(m_CurTime.QuadPart) - static_cast<double>(m_PrevTime.QuadPart)) / static_cast<double>(m_CountTime.QuadPart);

        m_fDeltaTime = static_cast<float>(m_dDeltaTime);
        m_PrevTime = m_CurTime;

        return m_fDeltaTime;
    }

    bool Frame_Limit(float frame_limit)
    {
        m_arrTimers_TimeAcc += update();

        if ((1.f / frame_limit) < m_arrTimers_TimeAcc)
        {
            //cout << "fps : " << 1.f / m_arrTimers_TimeAcc << endl;
            m_arrTimers_TimeAcc = 0.f;
            return true;
        }
        return false;
    }

private:
    LARGE_INTEGER   m_CountTime;
    LARGE_INTEGER    m_CurTime;
    LARGE_INTEGER   m_PrevTime;

    double          m_dDeltaTime;
    float           m_fDeltaTime;
    float           m_arrTimers_TimeAcc = 0;
};