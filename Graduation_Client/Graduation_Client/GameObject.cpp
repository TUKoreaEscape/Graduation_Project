#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"

std::string GameObject::OthersParts[5][6];
std::string GameObject::PlayerParts[6];

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void GameObject::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void GameObject::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

GameObject::GameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	//Scene::scene->creationQueue.push(this);
	renderer = new DeferredRenderer();
	renderer->gameObject = this;
}

GameObject::~GameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (renderer) renderer->Release();

	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
}

void GameObject::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	renderer->render(pd3dCommandList);
	if (m_pSibling) m_pSibling->render(pd3dCommandList);
	if (m_pChild) m_pChild->render(pd3dCommandList);
}

Texture* GameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	Texture* pTexture = nullptr;
	if (renderer) if (pTexture = renderer->FindReplicatedTexture(pstrTextureName)) return pTexture;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return pTexture;
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return pTexture;
	return nullptr;
}

void GameObject::SetMesh(Mesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void GameObject::SetChild(GameObject* pChild, bool bReferenceUpdate)
{
	if (pChild) {
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void GameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void GameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
	renderer->ReleaseUploadBuffers();

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void GameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void GameObject::SetDraw()
{
	isNotDraw = false;
	if (m_pSibling) m_pSibling->SetDraw();
	if (m_pChild) m_pChild->SetDraw();
}

void GameObject::SetParts(int player, int index, int partsNum)
{
	if (player == PLAYER) {
		switch (index) {
		case 0:
			PlayerParts[index] = Bodies[partsNum].c_str();
			break;
		case 1:
			PlayerParts[index] = Bodyparts[partsNum].c_str();
			break;
		case 2:
			PlayerParts[index] = Eyes[partsNum].c_str();
			break;
		case 3:
			PlayerParts[index] = Gloves[partsNum].c_str();
			break;
		case 4:
			PlayerParts[index] = MouthandNoses[partsNum].c_str();
			break;
		case 5:
			PlayerParts[index] = Head[partsNum].c_str();
			break;
		default:
			break;
		}
	}
	else {
		switch (index) {
		case 0:
			OthersParts[player - 1][index] = Bodies[partsNum].c_str();
			break;
		case 1:
			OthersParts[player - 1][index] = Bodyparts[partsNum].c_str();
			break;
		case 2:
			OthersParts[player - 1][index] = Eyes[partsNum].c_str();
			break;
		case 3:
			OthersParts[player - 1][index] = Gloves[partsNum].c_str();
			break;
		case 4:
			OthersParts[player - 1][index] = MouthandNoses[partsNum].c_str();
			break;
		case 5:
			OthersParts[player - 1][index] = Head[partsNum].c_str();
			break;
		default:
			break;
		}
	}
	
}

void GameObject::CacheSkinningBoneFrames(GameObject* pRootFrame)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		SkinnedMesh* pSkinnedMesh = (SkinnedMesh*)m_pMesh;
		for (int i = 0; i < pSkinnedMesh->m_nSkinningBones; i++)
		{
			pSkinnedMesh->m_ppSkinningBoneFrameCaches[i] = pRootFrame->FindFrame(pSkinnedMesh->m_ppstrSkinningBoneNames[i]);
#ifdef _WITH_DEBUG_SKINNING_BONE
			TCHAR pstrDebug[256] = { 0 };
			TCHAR pwstrBoneCacheName[64] = { 0 };
			TCHAR pwstrSkinningBoneName[64] = { 0 };
			size_t nConverted = 0;
			mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pSkinnedMesh->m_ppSkinningBoneFrameCaches[i]->m_pstrFrameName, _TRUNCATE);
			mbstowcs_s(&nConverted, pwstrSkinningBoneName, 64, pSkinnedMesh->m_ppstrSkinningBoneNames[i], _TRUNCATE);
			_stprintf_s(pstrDebug, 256, _T("SkinningBoneFrame:: Cache(%s) Bone(%s)\n"), pwstrBoneCacheName, pwstrSkinningBoneName);
			OutputDebugString(pstrDebug);
#endif
		}
	}
	if (m_pSibling) m_pSibling->CacheSkinningBoneFrames(pRootFrame);
	if (m_pChild) m_pChild->CacheSkinningBoneFrames(pRootFrame);
}

void GameObject::PrintFrameInfo(GameObject* pGameObject, GameObject* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) GameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) GameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);

}

GameObject* GameObject::FindFrame(char* pstrFrameName)
{
	GameObject* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

SkinnedMesh* GameObject::FindSkinnedMesh(char* pstrSkinnedMeshName)
{
	SkinnedMesh* pSkinnedMesh = NULL;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		pSkinnedMesh = (SkinnedMesh*)m_pMesh;
		if (!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}

void GameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void GameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void GameObject::LoadAnimationFromFile(FILE* pInFile, LoadedModelInfo* pLoadedModel)
{
	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nReads = (UINT)::fread(&nAnimationSets, sizeof(int), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			nReads = (UINT)::fread(&pLoadedModel->m_nSkinnedMeshes, sizeof(int), 1, pInFile);

			if (pLoadedModel->m_nSkinnedMeshes == 70) {
				pLoadedModel->m_pnAnimatedBoneFrames = new int;
				pLoadedModel->m_ppAnimationSets = new AnimationSets*;
				pLoadedModel->m_ppSkinnedMeshes = new SkinnedMesh*;
				pLoadedModel->m_pppAnimatedBoneFrameCaches = new GameObject**;

				pLoadedModel->m_pnAnimatedBoneFrames[0] = pLoadedModel->m_nSkinnedMeshes;
				pLoadedModel->m_pppAnimatedBoneFrameCaches[0] = new GameObject * [pLoadedModel->m_pnAnimatedBoneFrames[0]];

				::ReadStringFromFile(pInFile, pstrToken); //Skinned Mesh Name
				pLoadedModel->m_ppSkinnedMeshes[0] = (SkinnedMesh*)pLoadedModel->m_pModelRootObject->FindFrame(pstrToken)->m_pChild->m_pChild;
				pLoadedModel->m_ppSkinnedMeshes[0]->PrepareSkinning(pLoadedModel->m_pModelRootObject);
				for (int i = 0; i < pLoadedModel->m_pnAnimatedBoneFrames[0]; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken); //Skinned Mesh Name

					pLoadedModel->m_pppAnimatedBoneFrameCaches[0][i] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

				}
			}
			else {
				pLoadedModel->m_pnAnimatedBoneFrames = new int[pLoadedModel->m_nSkinnedMeshes];
				pLoadedModel->m_ppAnimationSets = new AnimationSets * [pLoadedModel->m_nSkinnedMeshes];
				pLoadedModel->m_ppSkinnedMeshes = new SkinnedMesh * [pLoadedModel->m_nSkinnedMeshes];
				pLoadedModel->m_pppAnimatedBoneFrameCaches = new GameObject * *[pLoadedModel->m_nSkinnedMeshes];

				for (int i = 0; i < pLoadedModel->m_nSkinnedMeshes; i++)
				{
					pLoadedModel->m_ppAnimationSets[i] = new AnimationSets(nAnimationSets);

					int nSkin = ::ReadIntegerFromFile(pInFile); //i

					::ReadStringFromFile(pInFile, pstrToken); //Skinned Mesh Name
					pLoadedModel->m_ppSkinnedMeshes[i] = pLoadedModel->m_pModelRootObject->FindSkinnedMesh(pstrToken);
					pLoadedModel->m_ppSkinnedMeshes[i]->PrepareSkinning(pLoadedModel->m_pModelRootObject);

					pLoadedModel->m_pnAnimatedBoneFrames[i] = ::ReadIntegerFromFile(pInFile);
					pLoadedModel->m_pppAnimatedBoneFrameCaches[i] = new GameObject * [pLoadedModel->m_pnAnimatedBoneFrames[i]];

					for (int j = 0; j < pLoadedModel->m_pnAnimatedBoneFrames[i]; j++)
					{
						::ReadStringFromFile(pInFile, pstrToken);
						pLoadedModel->m_pppAnimatedBoneFrameCaches[i][j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
						TCHAR pstrDebug[256] = { 0 };
						TCHAR pwstrAnimationBoneName[64] = { 0 };
						TCHAR pwstrBoneCacheName[64] = { 0 };
						size_t nConverted = 0;
						mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
						mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_pppAnimatedBoneFrameCaches[i][j]->m_pstrFrameName, _TRUNCATE);
						_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
						OutputDebugString(pstrDebug);
#endif
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			for (int i = 0; i < pLoadedModel->m_nSkinnedMeshes; i++)
			{
				pLoadedModel->m_ppAnimationSets[i]->m_ppAnimationSets[nAnimationSet] = new AnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pnAnimatedBoneFrames[i], pstrToken);
			}

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);
					for (int j = 0; j < pLoadedModel->m_nSkinnedMeshes; j++)
					{
						int nSkin = ::ReadIntegerFromFile(pInFile); //j
						AnimationSet* pAnimationSet = pLoadedModel->m_ppAnimationSets[j]->m_ppAnimationSets[nAnimationSet];
						pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
						nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pnAnimatedBoneFrames[j], pInFile);
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

GameObject* GameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObject* pParent, FILE* pInFile, Shader* pShader, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	GameObject* pGameObject = new GameObject();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			StandardMesh* pMesh = new StandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			SkinnedMesh* pSkinnedMesh = new SkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->renderer->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					GameObject* pChild = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

LoadedModelInfo* GameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	LoadedModelInfo* pLoadedModel = new LoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				GameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

void GameObject::Animate(float fTimeElapsed, int player)
{
	//OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this, player);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, player);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, player);
}

void GameObject::FindCustomPart(const char* pstrFrameName)
{
	if (!m_pChild) return;
	GameObject* pGameObject = m_pChild;
	while (pGameObject) {
		if (strncmp(pGameObject->m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) {
			pGameObject->isNotDraw = true;
		}
		pGameObject = pGameObject->m_pSibling;
	}
}
