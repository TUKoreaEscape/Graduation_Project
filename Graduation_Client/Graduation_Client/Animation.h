#pragma once
#include "stdafx.h"
#include "Object.h"
#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_PINGPONG		2

struct CALLBACKKEY
{
	float  							m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

//#define _WITH_ANIMATION_SRT		//애니메이션 행렬 대신에 SRT 정보를 사용
#define _WITH_ANIMATION_INTERPOLATION

class AnimationSet
{
public:
	AnimationSet();
	~AnimationSet();

public:
	char							m_pstrName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nAnimationBoneFrames = 0;

	int								m_nKeyFrameTransforms = 0;
	float* m_pfKeyFrameTransformTimes = NULL;
	XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

	float 							m_fSpeed = 1.0f;
	float 							m_fPosition = 0.0f;
	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int								m_nCurrentKey = -1;

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY* m_pCallbackKeys = NULL;

public:
	float GetPosition(float fPosition);
	XMFLOAT4X4 GetSRT(int nFrame, float fPosition);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);

	void* GetCallback(float fPosition) { return(NULL); }
};

class AnimationTrack
{
public:
	AnimationTrack() { }
	~AnimationTrack() { }

public:
	BOOL 							m_bEnable = true;
	float 							m_fSpeed = 0.1f;
	float 							m_fPosition = 0.0f;
	float 							m_fWeight = 1.0f;

	AnimationSet* m_pAnimationSet = NULL;
};

class AnimationCallbackHandler
{
public:
	virtual void HandleCallback(void* pCallbackData) { }
};

class AnimationController
{
public:
	AnimationController(int nAnimationTracks = 1);
	~AnimationController();

public:
	float 							m_fTime = 0.0f;

	int								m_nAnimationSets = 0;
	AnimationSet* m_pAnimationSets = NULL;

	int								m_nAnimationSet = 0;

	int								m_nAnimationBoneFrames = 0;
	Object** m_ppAnimationBoneFrameCaches = NULL;

	int 							m_nAnimationTracks = 0;
	AnimationTrack* m_pAnimationTracks = NULL;

	int  				 			m_nAnimationTrack = 0;

	Object* m_pRootFrame = NULL;

public:
	void SetAnimationSet(int nAnimationSet);

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void* pData);

	void AdvanceTime(float fElapsedTime, AnimationCallbackHandler* pCallbackHandler);
};