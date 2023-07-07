#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Game_state.h"
#include "protocol.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define DIR_NOT_FB				0x03
#define DIR_NOT_LR				0x0C
#define DIR_NOT_FBLR				0x0F

#define DIR_EMPTY					0x40

#define ROOM_COUNT			6

class Player : public GameObject
{
public:
	Player();

	virtual void update(float elapsedTime);
	//XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	//XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	//XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	//XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetShift(const XMFLOAT3& xmf3shift) { m_xmf3Shift = xmf3shift; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	
	
	void SetPosition(const XMFLOAT3& xmf3Position, bool is) { m_xmf3Position = xmf3Position; }
	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }
	void SetDirection(const DWORD direction) { m_direction = direction; }

	const XMFLOAT3& GetShift() const { return (m_xmf3Shift); }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	DWORD GetDirection() const { return (m_direction); }


	bool IsJump() { return m_JumpElapsedTime < m_Jumpping; }
	void SetJumpTime(float elapsedTime) { m_JumpElapsedTime += elapsedTime; }

	bool GetIsFalling() { return m_Isfalling; }
	void SetIsFalling(bool state) { m_Isfalling = state; }

	bool GetIsColledUpFace() { return m_collision_up_face; }
	void SetIsColledUpFace(bool state) { m_collision_up_face = state; }

	bool IsAttack() { return m_AttackElapsedTime < m_Attack; }
	void PlayAttack(float elapsedTime) { m_AttackElapsedTime += elapsedTime; }
	void SetAttackZeroTime() { m_AttackElapsedTime = 0.0f; }

	void ChangeCamera(GAME_STATE prev, GAME_STATE p);

	void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) {};
	void Rotate(float x, float y, float z);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext[ROOM_COUNT]);

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	/*CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);*/
	virtual void OnPrepareRender();
	Camera*						m_pCamera = NULL;

	void SetID(const int id) { m_id = id; }
	int GetID() const { return m_id; }

	void SetPlayerType(int type);

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f)) override;
protected:
	int							m_id = -1;
	char						name[64];
	//XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Shift = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, -0.981f * 2, 0.0f);
	float           			m_fMaxVelocityXZ = 15.f;
	float           			m_fMaxVelocityY = 10.0f;
	float           			m_fFriction = 100.0f;

	LPVOID						m_pPlayerUpdatedContext[ROOM_COUNT]{NULL};
	LPVOID						m_pCameraUpdatedContext = NULL;

	DWORD						m_direction;

	float							m_Jumpping = 0.3f;
	float							m_JumpElapsedTime = 0.f;

	float							m_Attack = 0.5f;
	float							m_AttackElapsedTime = 1.f;


	bool							m_Isfalling = false;
	bool							m_collision_up_face = false;

	int							m_playerType = 2;

public:
	GameObject* m_pNearDoor = nullptr;
	GameObject* m_pNearInteractionObejct = nullptr;
	GameObject* m_pNearVent = nullptr;
	GameObject* m_pNearItembox = nullptr;

	int							m_power_number = -1;
	int							m_vent_number = -1;
	int							m_itembox_number = -1;

	GAME_ITEM::ITEM				m_got_item = GAME_ITEM::ITEM_NONE;

	bool						m_bTaggerSkills[3];

	bool PickUpItem(GAME_ITEM::ITEM item);
	int GetItem();

	bool GetTaggerSkill(int index);
	void SetTaggerSkill(int index);
	bool UseTaggerSkill(int index);
};