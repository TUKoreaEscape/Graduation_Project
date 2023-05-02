#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Animation.h"

AnimationSet::AnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nSkinningBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;
	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nSkinningBones];

}

AnimationSet::~AnimationSet()
{
	if (m_pfKeyFrameTimes) delete[] m_pfKeyFrameTimes;
	for (int j = 0; j < m_nKeyFrames; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;

	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void AnimationSet::SetPosition(float fTrackPosition)
{
	m_fPosition = fTrackPosition;
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames - 1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
		//			m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
		//			m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	if (m_pAnimationCallbackHandler)
	{
		void* pCallbackData = GetCallbackData();
		if (pCallbackData) m_pAnimationCallbackHandler->HandleCallback(pCallbackData);
	}
}

XMFLOAT4X4 AnimationSet::GetSRT(int nBone)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nBone]), t);
			break;
		}
	}
	for (int i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nBone]), t);
			break;
		}
	}
	for (int i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone]), XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nBone]), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}
#endif
	return(xmf4x4Transform);
}

void AnimationSet::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void AnimationSet::SetCallbackKey(int nKeyIndex, float fKeyTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void AnimationSet::SetAnimationCallbackHandler(AnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void* AnimationSet::GetCallbackData()
{
	for (int i = 0; i < m_nCallbackKeys; i++)
	{
		if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON)) return(m_pCallbackKeys[i].m_pCallbackData);
	}
	return(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
AnimationSets::AnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_ppAnimationSets = new AnimationSet * [nAnimationSets];
}

AnimationSets::~AnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_ppAnimationSets[i]) delete m_ppAnimationSets[i];
	if (m_ppAnimationSets) delete[] m_ppAnimationSets;
}

void AnimationSets::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_ppAnimationSets[nAnimationSet]->m_nCallbackKeys = nCallbackKeys;
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void AnimationSets::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void* pData)
{
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;

}

void AnimationSets::SetAnimationCallbackHandler(int nAnimationSet, AnimationCallbackHandler* pCallbackHandler)
{
	m_ppAnimationSets[nAnimationSet]->SetAnimationCallbackHandler(pCallbackHandler);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

AnimationController::AnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, LoadedModelInfo* pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new AnimationTrack[nAnimationTracks];

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;

	m_ppAnimationSets = new AnimationSets * [m_nSkinnedMeshes];
	m_pnAnimatedBoneFrames = new int[m_nSkinnedMeshes];
	m_ppSkinnedMeshes = new SkinnedMesh * [m_nSkinnedMeshes];
	m_pppAnimatedBoneFrameCaches = new GameObject * *[m_nSkinnedMeshes];

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

		m_ppAnimationSets[i] = pModel->m_ppAnimationSets[i];
		m_ppAnimationSets[i]->AddRef();

		m_pnAnimatedBoneFrames[i] = pModel->m_pnAnimatedBoneFrames[i];

		m_pppAnimatedBoneFrameCaches[i] = new GameObject * [m_pnAnimatedBoneFrames[i]];
		for (int j = 0; j < m_pnAnimatedBoneFrames[i]; j++)
		{
			m_pppAnimatedBoneFrameCaches[i][j] = pModel->m_pppAnimatedBoneFrameCaches[i][j];
		}
	}

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256ÀÇ ¹è¼ö
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}

AnimationController::~AnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;
	if (m_pnAnimatedBoneFrames) delete[] m_pnAnimatedBoneFrames;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		if (m_ppAnimationSets[i]) m_ppAnimationSets[i]->Release();
		if (m_pppAnimatedBoneFrameCaches[i]) delete[] m_pppAnimatedBoneFrameCaches[i];

		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}

	if (m_pppAnimatedBoneFrameCaches) delete[] m_pppAnimatedBoneFrameCaches;
	if (m_ppAnimationSets) delete[] m_ppAnimationSets;

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;

	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

}

void AnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, int player)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		bool isNotUpdate = true;
		if (player == 0) {
			for (auto& str : GameObject::PlayerParts) {
				if (!strcmp(str.c_str(), m_ppSkinnedMeshes[i]->m_pstrMeshName)) {
					isNotUpdate = false;
					break;
				}
			}
		}
		else if (player != -1) {
			for (auto& str : GameObject::OthersParts[player - 1]) {
				if (!strcmp(str.c_str(), m_ppSkinnedMeshes[i]->m_pstrMeshName)) {
					isNotUpdate = false;
					break;
				}
			}
		}
		if (isNotUpdate)
			continue;
		m_ppSkinnedMeshes[i]->m_pd3dcbBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4BoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

void AnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
}

void AnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void AnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void AnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void AnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void AnimationController::SetCallbackKeys(int nSkinnedMesh, int nAnimationSet, int nCallbackKeys)
{
	if (m_ppAnimationSets && m_ppAnimationSets[nSkinnedMesh]) m_ppAnimationSets[nSkinnedMesh]->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void AnimationController::SetCallbackKey(int nSkinnedMesh, int nAnimationSet, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_ppAnimationSets && m_ppAnimationSets[nSkinnedMesh]) m_ppAnimationSets[nSkinnedMesh]->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, pData);
}

void AnimationController::SetAnimationCallbackHandler(int nSkinnedMesh, int nAnimationSet, AnimationCallbackHandler* pCallbackHandler)
{
	if (m_ppAnimationSets && m_ppAnimationSets[nSkinnedMesh]) m_ppAnimationSets[nSkinnedMesh]->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void AnimationController::AdvanceTime(float fTimeElapsed, GameObject* pRootGameObject, int player)
{
	m_fTime += fTimeElapsed;
	if (m_pAnimationTracks)
	{
		for (int i = 0; i < m_nAnimationTracks; i++) m_pAnimationTracks[i].m_fPosition += (fTimeElapsed * m_pAnimationTracks[i].m_fSpeed);

		for (int i = 0; i < m_nSkinnedMeshes; i++)
		{
			bool isNotUpdate = true;
			if (player == 0) {
				for (const std::string& str : GameObject::PlayerParts) {
					if (!strcmp(str.c_str(), m_ppSkinnedMeshes[i]->m_pstrMeshName)) {
						isNotUpdate = false;
						break;
					}
				}
			}
			else if (player != -1) {
				for (const std::string& str : GameObject::OthersParts[player - 1]) {
					if (!strcmp(str.c_str(), m_ppSkinnedMeshes[i]->m_pstrMeshName)) {
						isNotUpdate = false;
						break;
					}
				}
			}
			if (isNotUpdate)
				continue;
			for (int j = 0; j < m_pnAnimatedBoneFrames[i]; j++)
			{
				XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Zero();
				for (int k = 0; k < m_nAnimationTracks; k++)
				{
					if (m_pAnimationTracks[k].m_bEnable)
					{
						AnimationSet* pAnimationSet = m_ppAnimationSets[i]->m_ppAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
						pAnimationSet->SetPosition(m_pAnimationTracks[k].m_fPosition);
						XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j);
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
					}
				}
				m_pppAnimatedBoneFrameCaches[i][j]->m_xmf4x4ToParent = xmf4x4Transform;
			}
		}

		pRootGameObject->UpdateTransform(NULL);
	}
}

LoadedModelInfo::~LoadedModelInfo()
{
	if (m_ppAnimationSets) delete[] m_ppAnimationSets;
	if (m_pnAnimatedBoneFrames) delete[] m_pnAnimatedBoneFrames;
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		if (m_pppAnimatedBoneFrameCaches[i]) delete[] m_pppAnimatedBoneFrameCaches[i];
	}
	if (m_pppAnimatedBoneFrameCaches) delete[] m_pppAnimatedBoneFrameCaches;
}