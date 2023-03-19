#include "Object.h"

Object::Object()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

Object::Object(int nMaterials) : Object()
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new Material * [m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}

Object::~Object()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;
}

void Object::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void Object::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void Object::SetChild(Object* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
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

void Object::SetMesh(Mesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void Object::SetShader(Shader* pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new Material * [m_nMaterials];
	m_ppMaterials[0] = new Material(0);
	m_ppMaterials[0]->SetShader(pShader);
}

void Object::SetShader(int nMaterial, Shader* pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void Object::SetMaterial(int nMaterial, Material* pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

Object* Object::FindFrame(char* pstrFrameName)
{
	Object* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

Object* Object::GetRootSkinnedGameObject()
{
	if (m_pAnimationController) return(this);

	Object* pRootSkinnedGameObject = NULL;
	if (m_pSibling) if (pRootSkinnedGameObject = m_pSibling->GetRootSkinnedGameObject()) return(pRootSkinnedGameObject);
	if (m_pChild) if (pRootSkinnedGameObject = m_pChild->GetRootSkinnedGameObject()) return(pRootSkinnedGameObject);

	return(NULL);
}

void Object::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void Object::SetAnimationSet(int nAnimationSet)
{
	if (m_pAnimationController) m_pAnimationController->SetAnimationSet(nAnimationSet);

	if (m_pSibling) m_pSibling->SetAnimationSet(nAnimationSet);
	if (m_pChild) m_pChild->SetAnimationSet(nAnimationSet);
}

void Object::update(float fTimeElapsed)
{
	GameObject::update(fTimeElapsed);
	if (m_pAnimationController) m_pAnimationController->AdvanceTime(fTimeElapsed, NULL);

	if (m_pSibling) m_pSibling->update(fTimeElapsed);
	if (m_pChild) m_pChild->update(fTimeElapsed);
}

void Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, Camera* pCamera)
{
	OnPrepareRender();

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}
	}
	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}

void Object::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void Object::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void Object::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void Object::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, Material* pMaterial)
{
}

void Object::ReleaseShaderVariables()
{
}

void Object::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void Object::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void Object::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void Object::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 Object::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 Object::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 Object::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 Object::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void Object::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	Object::SetPosition(xmf3Position);
}

void Object::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	Object::SetPosition(xmf3Position);
}

void Object::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	Object::SetPosition(xmf3Position);
}

void Object::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void Object::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void Object::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

Texture* Object::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i])
		{
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{
				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
				}
			}
		}
	}
	Texture* pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

void Object::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Object* pParent, FILE* pInFile, Shader* pShader)
{
	char pstrToken[64] = { '\0' };

	int nMaterial = 0;
	BYTE nStrLength = 0;

	UINT nReads = (UINT)::fread(&m_nMaterials, sizeof(int), 1, pInFile);

	m_ppMaterials = new Material * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	Material* pMaterial = NULL;

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nReads = (UINT)::fread(&nMaterial, sizeof(int), 1, pInFile);

			pMaterial = new Material(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 3, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}

Object* Object::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Object* pParent, FILE* pInFile, Shader* pShader)
{
	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	Object* pGameObject = NULL;

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<Frame>:"))
		{
			pGameObject = new Object();

			nReads = (UINT)::fread(&nFrame, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&nTextures, sizeof(int), 1, pInFile);

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
			nReads = (UINT)::fread(pGameObject->m_pstrFrameName, sizeof(char), nStrLength, pInFile);
			pGameObject->m_pstrFrameName[nStrLength] = '\0';
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
			SkinnedMesh* pSkinnedMesh = new SkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
			nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile); //<Mesh>:
			pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = 0;
			nReads = (UINT)::fread(&nChilds, sizeof(int), 1, pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					Object* pChild = Object::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader);
					if (pChild) pGameObject->SetChild(pChild, false);
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

void Object::PrintFrameInfo(Object* pGameObject, Object* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) Object::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) Object::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void Object::LoadAnimationFromFile(FILE* pInFile)
{
	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nReads = (UINT)::fread(&m_pAnimationController->m_nAnimationSets, sizeof(int), 1, pInFile);

			m_pAnimationController->m_pAnimationSets = new AnimationSet[m_pAnimationController->m_nAnimationSets];
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			nReads = (UINT)::fread(&m_pAnimationController->m_nAnimationBoneFrames, sizeof(int), 1, pInFile);
			m_pAnimationController->m_ppAnimationBoneFrameCaches = new Object * [m_pAnimationController->m_nAnimationBoneFrames];

			for (int i = 0; i < m_pAnimationController->m_nAnimationBoneFrames; i++)
			{
				nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
				nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
				pstrToken[nStrLength] = '\0';

				m_pAnimationController->m_ppAnimationBoneFrameCaches[i] = FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, m_pAnimationController->m_ppAnimationBoneFrameCaches[i]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = 0;
			nReads = (UINT)::fread(&nAnimationSet, sizeof(int), 1, pInFile);
			AnimationSet* pAnimationSet = &m_pAnimationController->m_pAnimationSets[nAnimationSet];

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
			nReads = (UINT)::fread(pAnimationSet->m_pstrName, sizeof(char), nStrLength, pInFile);
			pAnimationSet->m_pstrName[nStrLength] = '\0';

			TCHAR pstrDebug[256] = { 0 };
			TCHAR pwstrAnimationBoneName[64] = { 0 };
			size_t nConverted = 0;
			mbstowcs_s(&nConverted, pwstrAnimationBoneName, nStrLength + 1, pAnimationSet->m_pstrName, _TRUNCATE);
			_stprintf_s(pstrDebug, 256, _T("AnimationSet - %s\n"), pwstrAnimationBoneName);
			OutputDebugString(pstrDebug);

			nReads = (UINT)::fread(&pAnimationSet->m_fLength, sizeof(float), 1, pInFile);
			nReads = (UINT)::fread(&pAnimationSet->m_nFramesPerSecond, sizeof(int), 1, pInFile);

			nReads = (UINT)::fread(&pAnimationSet->m_nKeyFrameTransforms, sizeof(int), 1, pInFile);
			pAnimationSet->m_pfKeyFrameTransformTimes = new float[pAnimationSet->m_nKeyFrameTransforms];
			pAnimationSet->m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [pAnimationSet->m_nKeyFrameTransforms];
			for (int i = 0; i < pAnimationSet->m_nKeyFrameTransforms; i++) pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[m_pAnimationController->m_nAnimationBoneFrames];

			for (int i = 0; i < pAnimationSet->m_nKeyFrameTransforms; i++)
			{
				nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
				nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
				pstrToken[nStrLength] = '\0';

				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					int nKeyFrame = 0;
					nReads = (UINT)::fread(&nKeyFrame, sizeof(int), 1, pInFile); //i

					nReads = (UINT)::fread(&pAnimationSet->m_pfKeyFrameTransformTimes[i], sizeof(float), 1, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(float), 16 * m_pAnimationController->m_nAnimationBoneFrames, pInFile);
				}
			}
#ifdef _WITH_ANIMATION_SRT
			nReads = (UINT)::fread(&pAnimationSet->m_nKeyFrameScales, sizeof(int), 1, pInFile);
			pAnimationSet->m_pfKeyFrameScaleTimes = new float[pAnimationSet->m_nKeyFrameScales];
			nReads = (UINT)::fread(&pAnimationSet->m_pfKeyFrameScaleTimes, sizeof(float), pAnimationSet->m_nKeyFrameScales, pInFile);
			pAnimationSet->m_ppxmf3KeyFrameScales = new XMFLOAT3 * [pAnimationSet->m_nKeyFrameScales];
			for (int i = 0; i < pAnimationSet->m_nKeyFrameScales; i++)
			{
				pAnimationSet->m_ppxmf3KeyFrameScales[i] = new XMFLOAT3[m_pAnimationController->m_nAnimationBoneFrames];
				nReads = (UINT)::fread(&pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), m_pAnimationController->m_nAnimationBoneFrames, pInFile);
			}
			nReads = (UINT)::fread(&pAnimationSet->m_nKeyFrameRotations, sizeof(int), 1, pInFile);
			pAnimationSet->m_pfKeyFrameRotationTimes = new float[pAnimationSet->m_nKeyFrameRotations];
			nReads = (UINT)::fread(&pAnimationSet->m_pfKeyFrameRotationTimes, sizeof(float), pAnimationSet->m_nKeyFrameRotations, pInFile);
			pAnimationSet->m_ppxmf3KeyFrameTranslations = new XMFLOAT3 * [pAnimationSet->m_nKeyFrameRotations];
			for (int i = 0; i < pAnimationSet->m_nKeyFrameRotations; i++)
			{
				pAnimationSet->m_ppxmf4KeyFrameRotations[i] = new XMFLOAT3[m_pAnimationController->m_nAnimationBoneFrames];
				nReads = (UINT)::fread(&pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT3), m_pAnimationController->m_nAnimationBoneFrames, pInFile);
			}
			nReads = (UINT)::fread(&pAnimationSet->m_nKeyFrameTranslations, sizeof(int), 1, pInFile);
			pAnimationSet->m_pfKeyFrameTranslationTimes = new float[pAnimationSet->m_nKeyFrameTranslations];
			nReads = (UINT)::fread(&pAnimationSet->m_pfKeyFrameTranslationTimes, sizeof(float), pAnimationSet->m_nKeyFrameTranslations, pInFile);
			pAnimationSet->m_ppxmf3KeyFrameTranslations = new XMFLOAT3 * [pAnimationSet->m_nKeyFrameTranslations];
			for (int i = 0; i < pAnimationSet->m_nKeyFrameTranslations; i++)
			{
				pAnimationSet->m_ppxmf3KeyFrameTranslations[i] = new XMFLOAT3[m_pAnimationController->m_nAnimationBoneFrames];
				nReads = (UINT)::fread(&pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), m_pAnimationController->m_nAnimationBoneFrames, pInFile);
			}
#endif
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

void Object::CacheSkinningBoneFrames(Object* pRootFrame)
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

Object* Object::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader, bool bHasAnimation)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	Object* pGameObject = Object::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader);

	pGameObject->CacheSkinningBoneFrames(pGameObject);

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	Object::PrintFrameInfo(pGameObject, NULL);
#endif

	if (bHasAnimation)
	{
		pGameObject->m_pAnimationController = new AnimationController(1);
		pGameObject->LoadAnimationFromFile(pInFile);
		pGameObject->m_pAnimationController->SetAnimationSet(0);
	}

	return(pGameObject);
}

HeightMapImage::HeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

HeightMapImage::~HeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 HeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float HeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

HeightMapTerrain::HeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : GameObject()
{
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	LPCTSTR pFileName = _T("Terrain/HeightMap.raw");
	m_nWidth = 257;
	m_nLength = 257;
	m_xmf3Scale = XMFLOAT3(8.0f, 2.0f, 8.0f);
	m_nMaterials = 1;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new Material * [m_nMaterials]; //°íÄ¥ºÎºÐ
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}

	m_pHeightMapImage = new HeightMapImage(pFileName, m_nWidth, m_nLength, m_xmf3Scale);

	HeightMapGridMesh* pMesh = new HeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, m_nWidth, m_nLength, m_xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	Texture* pTerrainBaseTexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainBaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Base_Texture.dds", RESOURCE_TEXTURE2D, 0);

	Texture* pTerrainDetailTexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainDetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_7.dds", RESOURCE_TEXTURE2D, 0);

	TerrainShader* pTerrainShader = new TerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	Scene::CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture, 0, 13);
	Scene::CreateShaderResourceViews(pd3dDevice, pTerrainDetailTexture, 0, 14);

	Material* pTerrainMaterial = new Material(2);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
	pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
	pTerrainMaterial->SetShader(pTerrainShader);

	SetMaterial(0, pTerrainMaterial);
}

HeightMapTerrain::~HeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}