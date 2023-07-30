#pragma once
#include "stdafx.h"
#include "Component.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Shader.h"
#include "GameObject.h"
#include "GameScene.h"
#include "protocol.h"

constexpr int DOOR_UI = 1;
constexpr int VENT_UI = 2;
constexpr int BOX_UI = 3;
constexpr int POWER_UI = 4;
constexpr int BLOCKED_UI = 5;
constexpr int INGAME_UI = 6;
constexpr int TAGGER_UI = 7;
constexpr int PROGRESS_BAR_UI = 8;

enum DIR {
	DEGREE0 = 0,
	DEGREE90,
	DEGREE180,
	DEGREE270
};

enum INFOUI {
	NOINFO = -1,
	FIXSUCCESS = 0,
	FIXFAIL,
	ACTIVATE_ESCAPESYSTEM,
	WORKING_ESCAPESYSTEM,
	ESCAPE_SUCCESS,
	SELECT_TAGGER
};

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

class InteractionUI : public GameObject
{
public:
	InteractionUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName);
	virtual ~InteractionUI();

	void BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList, DIR d, float gauge, int type);

	void Rotate(float fPitch, float fYaw, float fRoll) override;
	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type);
};

class InteractionObject : public GameObject
{
public:
	bool IsRot = false;
	bool IsNear = false;
	bool IsWorking = false;

	bool IsInteraction = false;

	InteractionUI** m_ppInteractionUIs = nullptr;
	int m_nUIs{};
	int m_nUIType = -1;

	float m_fPitch{}, m_fYaw{}, m_fRoll{};

	float volatile m_fCooltime{};

	DIR m_dir = DEGREE0;

	float m_fGauge{};

	bool m_bIsBlocked = false;
	bool m_bDoesOtherPlayerActive = false;

	int m_nSound = 0;
	int* m_pSounds = nullptr;
public:
	InteractionObject();
	virtual ~InteractionObject();
	virtual bool IsPlayerNear(const XMFLOAT3& PlayerPos) { return false; }
	virtual void Init() {}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void update(float fElapsedTime) {};

	virtual void Interaction(int playerType) override {};

	virtual void SetUI(int index, InteractionUI* ui);
	virtual void SetAnswer(int index, bool answer) {};
	virtual void SetSwitchValue(int index, bool value) {};
	virtual void SetIndex(int index) {};
	virtual void SetActivate(bool value) {};
	virtual void SetRotation(DIR d) {};

	virtual void SetBlock() { m_bIsBlocked = true; };
	virtual void SetUnBlock() { m_bIsBlocked = false; };

	virtual XMFLOAT3 GetPosition() { return(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43)); }

	virtual int GetDIR() const;

	virtual void CheckStart() { m_bDoesOtherPlayerActive = true; };
	virtual void CheckStop() { m_bDoesOtherPlayerActive = false; };
};

class Door : public InteractionObject
{
public:
	Door();
	virtual ~Door();

	void Rotate(float fPitch, float fYaw, float fRoll) override;
	void Init() override {};

public:
	float OpenTime = 0.0f;
	float TestTIme = 0.0f;
	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void update(float fElapsedTime) override;
	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;
	virtual void SetRotation(DIR d) override;

	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList);

	void Interaction(int playerType) override;

	XMFLOAT3 LeftDoorPos;
	XMFLOAT3 RightDoorPos;

	void SetOpen(bool Open);

	virtual bool GetIsWorking();
	int m_nDoorSoundIndex = -1;
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
	int	 m_switch_index = -1;

	bool m_bOnAndOff[10];
	GameObject* m_pCup = nullptr;
	GameObject* m_pMainKnob = nullptr;

	float m_fOffKnobPos;
	float m_fOnKnobPos;

	bool m_bClear = false;
	bool m_bIsOperating = false;

	float m_fCheckCooltime{};
	XMFLOAT4X4 m_xmf4x4MainKnobParent;

public:
	PowerSwitch();
	virtual ~PowerSwitch();
	
	void Init() override;
	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;

	void Rotate(float fPitch, float fYaw, float fRoll) override;
	virtual void SetRotation(DIR d) override;
	void SetOpen(bool Open);

	virtual void update(float fElapsedTime) override;
	void render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;

	void Interaction(int playerType) override;

	void PowerOperate();
	void SetIndex(int index) override;
	void SetAnswer(int index, bool answer) override;
	void SetSwitchValue(int index, bool value) override;
	void SetActivate(bool value) override;
	void OperateKnob(int index);
	bool CheckAnswer();
	void Reset();

	bool GetAnswer(int index) const { return m_bAnswers[index]; }
	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;

	void CheckStart() override;
	void CheckStop() override;
private:
	bool m_bAnswers[10];
};

class Vent : public InteractionObject
{
public:
	Vent();
	virtual ~Vent();

	void Rotate(float fPitch, float fYaw, float fRoll);

	void SetOpen(bool open);
	void SetOpenPos(const XMFLOAT3& pos);

	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;

	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void update(float fElapsedTime) override;

	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) override;

	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;
	virtual void SetRotation(DIR d) override;
	void Interaction(int playerType) override;
public:
	XMFLOAT3 m_xmf3OpenPosition;
	XMFLOAT3 m_xmf3ClosePosition;
};

class Item : public InteractionObject
{
public:
	Item();
	virtual ~Item();

	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;

	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;

	void Interaction(int playerType) override;
public:
	int m_ItemType = -1;
	bool m_bShow = false;
};

class ItemBox : public InteractionObject
{
public:
	ItemBox();
	virtual ~ItemBox();

	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;
	void Rotate(float fPitch, float fYaw, float fRoll);
	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void update(float fElapsedTime) override;
	virtual void Depthrender(ID3D12GraphicsCommandList* pd3dCommandList) override;

	void Interaction(int playerType) override;
	void SetOpen(bool open) override;

	virtual void SetRotation(DIR d) override;
	void SetIndex(int index) { m_item_box_index = index; }

	void SetItem(GAME_ITEM::ITEM item);
	void InitItems(int index, GameObject* item);
	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;
public:
	int m_item_box_index = -1;
	GameObject* m_pItems[6];
	GAME_ITEM::ITEM m_item = GAME_ITEM::ITEM::ITEM_NONE;

	bool m_bShownItem = false;

	float m_fPickupCooltime{};

	XMFLOAT4X4 m_xmf4x4CapMatrix;
	XMFLOAT4X4 m_xmf4x4CapOpenMatrix;
};

class IngameUI : public GameObject
{
public:
	int m_UIType = INGAME_UI;
	float m_fGauge{1};
public:
	IngameUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
	virtual ~IngameUI();

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type);
	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UIrender(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type);

	void SetGuage(float f) { m_fGauge = f; };
	void SetUIType(int type) { m_UIType = type; };

	void SetAnswer(bool on);
};

class MinimapUI : public GameObject
{
public:
	MinimapUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
	virtual ~MinimapUI();

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	void UIrender(ID3D12GraphicsCommandList* pd3dCommandList);
};

class TaggersBox : public InteractionObject
{
public:
	TaggersBox();
	virtual ~TaggersBox();

	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;
	void Rotate(float fPitch, float fYaw, float fRoll);
	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void update(float fElapsedTime) override;

	void Interaction(int playerType) override;
	void SetOpen(bool open) override;

	virtual void SetRotation(DIR d) override;
	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;
public:
	int m_nLifeChips{};
	bool m_bActivate{};

	void CollectChip();
	void Reset();
	void Init();
public:
	int m_nChips;
	GameObject** m_ppChips;
};

class EscapeObject : public InteractionObject
{
private:
	int	m_escapeobject_id = -1;
public:
	EscapeObject();
	virtual ~EscapeObject();

	void Init() override;
	bool IsPlayerNear(const XMFLOAT3& PlayerPos) override;
	void Rotate(float fPitch, float fYaw, float fRoll);
	void render(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void update(float fElapsedTime) override;

	void Interaction(int playerType) override;
	void SetOpen(bool open) override;

	virtual void SetRotation(DIR d) override;
	virtual void SetPosition(XMFLOAT3 xmf3Position) override;
	virtual void SetPosition(float x, float y, float z) override;
public:
	bool m_bIsReal = false;
	void SetID(int id) { m_escapeobject_id = id; }
	int	 GetID() { return m_escapeobject_id; }
	void SetReal() { m_bIsReal = true; };
	void Init_By_GameEnd() { m_bIsReal = false; IsWorking = false; }
	
	void SetWorking();

	float m_fCheckCooltime{};
	GameObject* m_pArm = nullptr;
	XMFLOAT4X4 m_xmf4x4ArmParent;
};

class PvsRoom : public GameObject
{
public:
	PvsRoom();
	virtual ~PvsRoom();

public:
	int m_nPvs{};
	void SetPvsNum(int num) { m_nPvs = num; }
	GameObject** m_ppPvs = nullptr;
	void SetPvs(int num);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class ParticleObject : public GameObject
{
public:
	ParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles);
	virtual ~ParticleObject();

	Texture* m_pRandowmValueTexture = NULL;
	Texture* m_pRandowmValueOnSphereTexture = NULL;

	void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void OnPostRender();
};
