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
// PCSS를 사용한 그림자 셰이더 예시
float PCSS_ShadowSampler(float3 lightPosition, float2 texCoord, float2 shadowMapSize, float shadowMapBias, float maxDistance)
{
    float2 moments = tex2D(PCF_ShadowMapSampler, texCoord).rg; // 그림자 맵의 샘플 깊이 값과 뎁스 모멘트
    float depth = tex2D(ShadowMapSampler, texCoord).r; // 깊이 값을 가져옴

    float searchRadius = maxDistance * (texCoord.z - shadowMapBias); // 샘플링 반경 설정
    float numSamples = 16.0f; // 샘플 수

    float penumbra = searchRadius / shadowMapSize.x; // 페넘브라 크기 계산

    float blockerDepth = moments.x; // 블로커 깊이값
    float sumBlockerDepth = moments.y; // 블로커 깊이값 합
    float averageBlockerDepth = sumBlockerDepth / numSamples; // 블로커 깊이값 평균

    // 그림자의 경계 판단
    float penumbraTest = (depth - averageBlockerDepth) / penumbra;
    float shadowFactor = clamp(penumbraTest, 0.0f, 1.0f);

    return shadowFactor;
}

*/