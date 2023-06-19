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

class DoorUI : public GameObject
{
public:
	DoorUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName);
	virtual ~DoorUI();

	void BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList);

	void Rotate(float fPitch, float fYaw, float fRoll) override;

};

class InteractionUI : public GameObject
{
public:
	InteractionUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName);
	virtual ~InteractionUI();

	void BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList);

	void Rotate(float fPitch, float fYaw, float fRoll) override;

};

class InteractionObject : public GameObject
{
public:
	bool IsNear = false;
	bool IsWorking = false;

	InteractionUI* m_pInteractionUI = nullptr;
public:
	InteractionObject();
	virtual ~InteractionObject();
	virtual bool IsPlayerNear(const XMFLOAT3& PlayerPos) { return false; }
	virtual void Init() {}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList) {};
};

class Door : public InteractionObject
{
public:
	Door();
	virtual ~Door();

	void Rotate(float fPitch, float fYaw, float fRoll) override;
	void Init() override {};

public:
	bool IsRot = false;
	bool IsNear = false;
	bool IsWorking = false;

	float OpenTime = 0.0f;
	float TestTIme = 0.0f;
	bool CheckDoor(const XMFLOAT3& PlayerPos);

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void update(float fElapsedTime);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList);

	XMFLOAT3 LeftDoorPos;
	XMFLOAT3 RightDoorPos;

	DoorUI* m_pDoorUI = nullptr;

	float m_fPitch{}, m_fYaw{}, m_fRoll{};

	void SetOpen(bool Open);

	virtual bool GetIsWorking();
};

class UIObject : public GameObject
{
public:
	UIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
	virtual ~UIObject();
};

class PowerSwitch : public InteractionObject
{
public:
	bool m_bOnAndOff[15];
	GameObject* m_pCup = nullptr;
	GameObject* m_pMainKnob = nullptr;

	float m_fOffKnobPos;
	float m_fOnKnobPos;

	bool m_bClear = false;

public:
	PowerSwitch();
	virtual ~PowerSwitch();
	
	void Init() override;
	bool IsPlayerNear(const XMFLOAT3& PlayerPos);

	void Rotate(float fPitch, float fYaw, float fRoll) override;

	void SetOpen(bool Open);

	void render(ID3D12GraphicsCommandList* pd3dCommandList);
};