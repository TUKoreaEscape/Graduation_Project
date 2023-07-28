#pragma once
#include "stdafx.h"
#include "Input.h"

extern RECT_FLOAT idRect;
extern RECT_FLOAT passwordRect;

extern RECT_FLOAT logininfoRect[5];

extern RECT_FLOAT roominfoRect[6];

extern RECT_FLOAT roomPageRect[2];

extern RECT_FLOAT pageNumRect;

extern RECT_FLOAT waitingRoomRect[3];

extern RECT_FLOAT customizingRect[10];

extern RECT_FLOAT endingRect;

extern RECT_FLOAT powerRect;

extern RECT_FLOAT chatBoxRect;

void UpdateRectSize(HWND hWnd);

/*
// PCSS�� ����� �׸��� ���̴� ����
float PCSS_ShadowSampler(float3 lightPosition, float2 texCoord, float2 shadowMapSize, float shadowMapBias, float maxDistance)
{
    float2 moments = tex2D(PCF_ShadowMapSampler, texCoord).rg; // �׸��� ���� ���� ���� ���� ���� ���Ʈ
    float depth = tex2D(ShadowMapSampler, texCoord).r; // ���� ���� ������

    float searchRadius = maxDistance * (texCoord.z - shadowMapBias); // ���ø� �ݰ� ����
    float numSamples = 16.0f; // ���� ��

    float penumbra = searchRadius / shadowMapSize.x; // ��Ѻ�� ũ�� ���

    float blockerDepth = moments.x; // ���Ŀ ���̰�
    float sumBlockerDepth = moments.y; // ���Ŀ ���̰� ��
    float averageBlockerDepth = sumBlockerDepth / numSamples; // ���Ŀ ���̰� ���

    // �׸����� ��� �Ǵ�
    float penumbraTest = (depth - averageBlockerDepth) / penumbra;
    float shadowFactor = clamp(penumbraTest, 0.0f, 1.0f);

    return shadowFactor;
}

*/