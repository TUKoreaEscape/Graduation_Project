#pragma once
#include "stdafx.h"
#include "Movement.h"
#include "Camera.h"
#include "Input.h"
#include "Player.h"
#include "Object.h"
#include "Network.h"
#include "Game_state.h"
#include "Sound.h"

Player::Player() : GameObject()
{
	//AddComponent<CommonMovement>();
	//GetComponent<CommonMovement>()->m_pPlayer = this;//movenment에서 player변수가 있는데 이걸 넘겨줘야한다.
	//AddComponent<FirstPersonCamera>();
	//GetComponent<FirstPersonCamera>()->targetObject = this; //마찬가지로 camera에서도 player변수가 있음
	//m_pCamera = GetComponent<FirstPersonCamera>();//플레이어가 사용하는 카메라를 지정해줘야한다.
	//AddComponent<StandardRenderer>();


	//m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	//m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;

	Input::GetInstance()->m_pPlayer = this;
	AddComponent<OtherPlayerCamera>();
	m_pCamera = GetComponent<OtherPlayerCamera>();
	m_pCamera->m_pPlayer = this;

	for (int i = 0; i < 3; ++i) {
		m_bTaggerSkills[i] = false;
	}
}

void Player::ChangeCamera(GAME_STATE prev, GAME_STATE p)
{
	m_xmf3PrevRight = m_xmf3Right;
	m_xmf3PrevUp = m_xmf3Up;
	m_xmf3PrevLook = m_xmf3Look;
	m_fPrevPitch = m_fPitch;
	m_fPrevRoll = m_fRoll;
	m_fPrevYaw = m_fYaw;

	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	switch (p) {
	case LOGIN:
		break;
	case ROOM_SELECT:
		break;
	case WAITING_GAME:
		if (prev == CUSTOMIZING)
		{
			DeleteComponent<CustomizingCamera>();
		}
		else if (prev == ENDING_GAME)
		{
			DeleteComponent<ThirdPersonCamera>();
		}
		AddComponent<ThirdPersonCamera>();
		m_pCamera = GetComponent<ThirdPersonCamera>();
		m_pCamera->m_pPlayer = this;
		break;
	case CUSTOMIZING:
		DeleteComponent<ThirdPersonCamera>();
		AddComponent<CustomizingCamera>();
		m_pCamera = GetComponent<CustomizingCamera>();
		m_pCamera->m_pPlayer = this;
		break;
	case READY_TO_GAME:
		DeleteComponent<ThirdPersonCamera>();
		AddComponent<FirstPersonCamera>();
		m_pCamera = GetComponent<FirstPersonCamera>();
		m_pCamera->m_pPlayer = this;
		break;
	case PLAYING_GAME:
		break;
	case ENDING_GAME:
		DeleteComponent<FirstPersonCamera>();
		AddComponent<ThirdPersonCamera>();
		m_pCamera = GetComponent<ThirdPersonCamera>();
		m_pCamera->m_pPlayer = this;
		break;
	case INTERACTION_POWER:
		break;
	}
}

void Player::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD	)	xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD)	xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT)			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT)				xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP)				xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance*20.0f);
		if (dwDirection & DIR_DOWN)			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		
		m_direction = dwDirection;
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void Player::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
#if !USE_NETWORK
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
#endif
#if USE_NETWORK
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_xmf3Shift = xmf3Shift;
#endif
		m_pCamera->Move(xmf3Shift);
	}

	//if (this == Input::GetInstance()->m_pPlayer)
	//{
	//	std::cout << "같음" << std::endl;
	//}
}

void Player::Rotate(float x, float y, float z)
{
	/*DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}*/
	/*else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}*/
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}
	m_pCamera->Rotate(x, y, z);
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	/*Network& server = *Network::GetInstance();
	cs_packet_player_rotate rotate_packet;
	rotate_packet.size = sizeof(rotate_packet);
	rotate_packet.type = CS_PACKET::CS_ROTATE;
	rotate_packet.xmf3Look = m_xmf3Look;
	rotate_packet.xmf3Up = m_xmf3Up;
	rotate_packet.xmf3Right = m_xmf3Right;
	rotate_packet.yaw = m_fYaw;
	server.send_packet(&rotate_packet);*/
}

void Player::update(float fTimeElapsed)
{
	//OnPrepareRender();

	GameObject::update(fTimeElapsed);
	m_fBlendingTime += fTimeElapsed;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	if (m_xmf3Position.y > 0.0f && !m_collision_up_face)
	{
		//std::cout << "중력작용중" << std::endl;
		//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
#if USE_NETWORK
		Network& network = *Network::GetInstance();

		if (network.m_pPlayer_before_Pos.y >= network.m_pPlayer_Pos.y)
			m_JumpElapsedTime = 0.3f;
#endif
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
		m_Isfalling = true;
	}
	else if (m_xmf3Position.y <= 0.0f || m_collision_up_face)
	{
		if (!m_collision_up_face)
			m_xmf3Position.y = 0;

		if (!IsJump())
			m_JumpElapsedTime = 0.0;
		m_Isfalling = false;
		m_pSkinnedAnimationController->SetTrackSpeed(0, 1);
	}
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false); // 이게 중요한듯?
#if !USE_NETWORK
	Move(xmf3Velocity, false);
#endif
	m_xmf3Shift = xmf3Velocity;
	if (m_pPlayerUpdatedContext[0]) OnPlayerUpdateCallback(fTimeElapsed);

	m_pCamera->update(fTimeElapsed);
	/*DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->update(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);*/
	m_pCamera->RegenerateViewMatrix();
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	if (m_bIsBlending) {
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		if (m_fBlendingTime > 0.33f) {
			m_nPrevAnimation = m_nNextAnimation;
			m_bIsBlending = false;
		}
	}
	else {
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackWeight(0, 1.0f);
	}
}

void Player::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Player::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void Player::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;
	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);

	//UpdateTransform(NULL);

	SetDraw();

	if (PlayerNum == 0) {
		GameObject* pp = FindFrame("Bodies");
		if (pp)
			pp->FindCustomPart(PlayerParts[0].c_str());
		pp = FindFrame("Bodyparts");
		if (pp)
			pp->FindCustomPart(PlayerParts[1].c_str());
		pp = FindFrame("Eyes");
		if (pp)
			pp->FindCustomPart(PlayerParts[2].c_str());
		pp = FindFrame("Gloves");
		if (pp)
			pp->FindCustomPart(PlayerParts[3].c_str());
		pp = FindFrame("MouthandNoses");
		if (pp)
			pp->FindCustomPart(PlayerParts[4].c_str());
		pp = FindFrame("head");
		if (pp)
			pp->FindCustomPart(PlayerParts[5].c_str());
	}
	else if (PlayerNum != -1) {
		GameObject* pp = FindFrame("Bodies");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][0].c_str());
		pp = FindFrame("Bodyparts");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][1].c_str());
		pp = FindFrame("Eyes");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][2].c_str());
		pp = FindFrame("Gloves");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][3].c_str());
		pp = FindFrame("MouthandNoses");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][4].c_str());
		pp = FindFrame("head");
		if (pp)
			pp->FindCustomPart(OthersParts[PlayerNum - 1][5].c_str());
	}

	SetType(m_playerType);
}

void Player::SetPlayerType(int type)
{
	m_playerType = type;
	m_Type = type;
}

void Player::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (PlayerNum != 0 && GameState::GetInstance()->GetGameState() == CUSTOMIZING) return;
	if (USE_NETWORK)
		if (GameState::GetInstance()->GetGameState() == WAITING_GAME)
			if (m_id == -1)
				return;

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList, PlayerNum);

	if (GameState::GetInstance()->GetGameState() == ENDING_GAME) {
#if USE_NETWORK
		Network& network = *Network::GetInstance();
		if(network.m_tagger_win){
#endif
#if !USE_NETWORK
		if (0) { // Tagger's Win
#endif
			if (this->GetType() == TYPE_TAGGER) {
				renderer->render(pd3dCommandList);
				if (m_pSibling) m_pSibling->render(pd3dCommandList);
				if (m_pChild) m_pChild->render(pd3dCommandList);
			}
		}
		else {
			if (this->GetType() == TYPE_PLAYER || this->GetType() == TYPE_DEAD_PLAYER) {
				renderer->render(pd3dCommandList);
				if (m_pSibling) m_pSibling->render(pd3dCommandList);
				if (m_pChild) m_pChild->render(pd3dCommandList);
			}
		}
		return;
	}

	if (PlayerNum == 0) {
		if (GameState::GetInstance()->GetGameState() > CUSTOMIZING) {
			GameObject* Hand = FindFrame("Gloves");
			Hand->m_pChild->render(pd3dCommandList);
		}
		else {
			renderer->render(pd3dCommandList);
			if (m_pSibling) m_pSibling->render(pd3dCommandList);
			if (m_pChild) m_pChild->render(pd3dCommandList);
		}
	}
	else {
		renderer->render(pd3dCommandList);
		if (m_pSibling) m_pSibling->render(pd3dCommandList);
		if (m_pChild) m_pChild->render(pd3dCommandList);
	}
}
	void Player::Depthrender(ID3D12GraphicsCommandList* pd3dCommandList)
	{
		renderer->Depthrender(pd3dCommandList);
		if (m_pSibling) m_pSibling->Depthrender(pd3dCommandList);
		if (m_pChild) m_pChild->Depthrender(pd3dCommandList);
	}


void Player::SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& xmf3Up)
{
	XMFLOAT3 position = m_xmf3Position;
	if (!IsEqual(position.y, 0)) {
		position.y = 0;
	}
	XMFLOAT3 target = xmf3Target;
	target.y = 0;
	XMFLOAT3 look = Vector3::Subtract(target, position);
	m_xmf3Look = Vector3::Normalize(look);
	m_xmf3Up = xmf3Up;
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_fPitch = 0; m_fRoll = 0; m_fYaw = 0;
	//m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	if (m_pCamera) m_pCamera->SetLookAt(xmf3Target);
}

bool Player::PickUpItem(GAME_ITEM::ITEM item)
{
	if (item == GAME_ITEM::ITEM_NONE) return false;
	if (item == GAME_ITEM::ITEM_LIFECHIP) {
		if (GetType() == TYPE_DEAD_PLAYER) {
			SetPlayerType(TYPE_PLAYER);
			return true;
		}
		return false;
	}
	if (m_got_item != GAME_ITEM::ITEM_NONE) return false;
	m_got_item = item;
	return true;
}

int Player::GetItem()
{
	switch (m_got_item) {
	case GAME_ITEM::ITEM_HAMMER:
		return 0;
	case GAME_ITEM::ITEM_DRILL:
		return 1;
	case GAME_ITEM::ITEM_WRENCH:
		return 2;
	case GAME_ITEM::ITEM_PLIERS:
		return 3;
	case GAME_ITEM::ITEM_DRIVER:
		return 4;
	default:
		return -1;
	}
}

bool Player::GetTaggerSkill(int index)
{
	if (GetType() != TYPE_TAGGER) return false;
	return m_bTaggerSkills[index];
}

void Player::SetTaggerSkill(int index)
{
	if (GetType() != TYPE_TAGGER) return;
	m_bTaggerSkills[index] = true;
}

bool Player::UseTaggerSkill(int index)
{
	if (GetType() != TYPE_TAGGER) return false;
	if (false == m_bTaggerSkills[index]) return false;
	m_bTaggerSkills[index] = false;
	// Do Skill here maybe
	return true;
}

void Player::SetAnimation(int index)
{
	m_nNextAnimation = index;
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, m_nPrevAnimation);
	m_pSkinnedAnimationController->SetTrackWeight(1, 0.7f);
	m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, index);
	m_pSkinnedAnimationController->SetTrackWeight(0, 0.3f);
	if (m_nPrevAnimation == index) return;
	else if (index > RUN_RIGHT) return;
	else {
		if (!m_bIsBlending)	m_fBlendingTime = 0;
		m_bIsBlending = true;
	}
}

void Player::ChangeSpectator()
{
	SpectatorPlayerIndex = (SpectatorPlayerIndex + 1) % 5;
}

void Player::SetAnimationCallback(int index)
{
	AnimationCallbackHandler* pAnimationCallbackHandler = new SoundCallbackHandler();
	channelIndex = index;
	if (m_pSkinnedAnimationController) {
		FootstepCallback1 = Sound::GetInstance()->CreatePlayersSounds("Sound/Footstep01.wav", index);
		FootstepCallback2 = Sound::GetInstance()->CreatePlayersSounds("Sound/Footstep02.wav", index);
		m_pSkinnedAnimationController->SetCallbackKeys(0, 1, 2);
		m_pSkinnedAnimationController->SetCallbackKey(0, 1, 0, 0.166f, &FootstepCallback1, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKey(0, 1, 1, 0.5f, &FootstepCallback2, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKeys(0, 2, 2);
		m_pSkinnedAnimationController->SetCallbackKey(0, 2, 0, 0.166f, &FootstepCallback1, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKey(0, 2, 1, 0.5f, &FootstepCallback2, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKeys(0, 3, 2);
		m_pSkinnedAnimationController->SetCallbackKey(0, 3, 0, 0.166f, &FootstepCallback1, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKey(0, 3, 1, 0.5f, &FootstepCallback2, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKeys(0, 4, 2);
		m_pSkinnedAnimationController->SetCallbackKey(0, 4, 0, 0.166f, &FootstepCallback1, &channelIndex);
		m_pSkinnedAnimationController->SetCallbackKey(0, 4, 1, 0.5f, &FootstepCallback2, &channelIndex);
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(0, 1, pAnimationCallbackHandler);
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(0, 2, pAnimationCallbackHandler);
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(0, 3, pAnimationCallbackHandler);
		m_pSkinnedAnimationController->SetAnimationCallbackHandler(0, 4, pAnimationCallbackHandler);
	}
}

void Player::ReleaseShaderVariables()
{
	//if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void Player::OnPlayerUpdateCallback(float fTimeElapsed)
{
	for (int i = 0; i < ROOM_COUNT; ++i)
	{
		HeightMapTerrain* pTerrain = (HeightMapTerrain*)m_pPlayerUpdatedContext[i];
		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
		bool bReverseQuad = ((z % 2) != 0);
		float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
		if (xmf3PlayerPosition.y < fHeight)
		{
			XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
			xmf3PlayerVelocity.y = 0.0f;
			SetVelocity(xmf3PlayerVelocity);
			xmf3PlayerPosition.y = fHeight;
			SetPosition(xmf3PlayerPosition);
			m_JumpElapsedTime = 0.0f;
			m_Isfalling = false;
			m_pSkinnedAnimationController->SetTrackSpeed(0, 1);
		}
	}
}

void Player::SetPlayerUpdatedContext(LPVOID pContext[ROOM_COUNT])
{
	if (pContext)
	{
		for (int i = 0; i < ROOM_COUNT; ++i)
		{
			m_pPlayerUpdatedContext[i] = pContext[i];
		}
	}
}
