#pragma once
#include "stdafx.h"
#include "Component.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Shader.h"
#include "GameObject.h"
#include "GameScene.h"

class SkyBox : public GameObject
{
public:
	SkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature);
	virtual ~SkyBox();
};

class HeightMapTerrain : public GameObject
{
public:
	HeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int startX, int startZ, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, wchar_t* pstrFileName);
	virtual ~HeightMapTerrain();

private:
	HeightMapImage* m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};

class Vent : public GameObject
{
public:
	Vent();
	virtual ~Vent();

	void Rotate(float fPitch, float fYaw, float fRoll);
};

class Door : public GameObject
{
public:
	Door();
	virtual ~Door();

	void Rotate(float fPitch, float fYaw, float fRoll);
};

class UIObject : public GameObject
{
public:
	UIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
	virtual ~UIObject();
};