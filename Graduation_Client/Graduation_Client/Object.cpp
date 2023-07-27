#include "Object.h"
#include "Input.h"
#include "Sound.h"

SkyBox::SkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	//Scene::scene->creationQueue.push(this);
	renderer = new SkyboxRenderer();
	renderer = static_cast<SkyboxRenderer*>(renderer);
	renderer->gameObject = this;

	SkyBoxMesh* pSkyBoxMesh = new SkyBoxMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pSkyBoxMesh);

	Texture* pSkyBoxTexture = new Texture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/riots_sru_skybox_cubemap.dds", RESOURCE_TEXTURE_CUBE, 0);
	
	DXGI_FORMAT pdxgiRtvFormats[6] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32_FLOAT };
	SkyBoxShader* pSkyBoxShader = new SkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 6, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	GameScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, 10);

	Material* pSkyBoxMaterial = new Material(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);
	renderer->start(pd3dDevice, pd3dCommandList);
	renderer->SetMaterial(0, pSkyBoxMaterial);
}

SkyBox::~SkyBox()
{
}


HeightMapTerrain::HeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int startX, int startZ, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, wchar_t* pstrFileName) : GameObject()
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	m_pHeightMapImage = new HeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	HeightMapGridMesh* pMesh = new HeightMapGridMesh(pd3dDevice, pd3dCommandList, startX - float(nWidth / 2), startZ - float(nLength / 2), nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	renderer->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	Texture* pTerrainBaseTexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainBaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	Texture* pTerrainDetailTexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainDetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_7.dds", RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture, 0, 13);
	GameScene::CreateShaderResourceViews(pd3dDevice, pTerrainDetailTexture, 0, 14);

	Material* pTerrainMaterial = new Material(2);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
	pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
	pTerrainMaterial->SetTerrainShader();

	renderer->SetMaterial(0, pTerrainMaterial);
}

HeightMapTerrain::~HeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

Vent::Vent() : InteractionObject()
{
	m_nUIs = 2;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];	
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	m_nUIType = VENT_UI;

	m_nSound = 1;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/Vent.mp3", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 20.0f, 50.0f);
}

Vent::~Vent()
{
	delete[] m_pSounds;
}

void Vent::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);

	IsRot = true;
}

void Vent::SetOpen(bool open)
{
	if (true == open) {
		if (IsOpen) return;
		Rotate(90, 0, 0);
		SetPosition(m_xmf3OpenPosition);
		UpdateTransform(NULL);
		IsOpen = true;
		m_fCooltime = 0;
		Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
	}
	else {
		if (IsOpen == false) return;
		Rotate(-90, 0, 0);
		SetPosition(m_xmf3ClosePosition);
		UpdateTransform(NULL);
		IsOpen = false;
		Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
	}
}

void Vent::SetOpenPos(const XMFLOAT3& pos)
{
	m_xmf3ClosePosition = m_xmf3Position;
	m_xmf3OpenPosition = pos;
}

bool Vent::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 1.75f;
		maxz = m_xmf4x4ToParent._43 + 1.75f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 1.75f;
		maxx = m_xmf4x4ToParent._41 + 1.75f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 1.75f;
		maxz = m_xmf4x4ToParent._43 + 1.75f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 1.75f;
		maxx = m_xmf4x4ToParent._41 + 1.75f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	}

	if (PlayerPos.x > maxx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.x < minx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z < minz) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z > maxz) {
		IsNear = false;
		return false;
	}
	IsNear = true;
	return true;
}

void Vent::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	GameObject::render(pd3dCommandList);
}

void Vent::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (IsOpen) return;
	if (Input::GetInstance()->m_pPlayer->m_Type == TYPE_TAGGER) return;
	if (IsNear) {
		if (m_bIsBlocked) {
			if (m_ppInteractionUIs[1]) {
				if (m_dir == DEGREE90 || m_dir == DEGREE270)
					m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.0f, m_xmf4x4ToParent._43);
				else
					m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
				m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, 1.0f, BLOCKED_UI);
			}
		}
		else {
			if (m_ppInteractionUIs[0]) {
				if (m_dir == DEGREE90 || m_dir == DEGREE270)
					m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.0f, m_xmf4x4ToParent._43);
				else
					m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
				m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
			}
		}
	}
}

void Vent::Move(float fxOffset, float fyOffset, float fzOffset)
{
}

void Vent::SetPosition(XMFLOAT3 xmf3Position)
{
	m_xmf3Position = xmf3Position;
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	Sound& sound = *Sound::GetInstance();
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void Vent::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x,y,z);
	GameObject::SetPosition(x,y,z);
	Sound& sound = *Sound::GetInstance();
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void Vent::Interaction(int playerType)
{
	if (true == m_bIsBlocked) return;
	if (true == IsOpen) return;
	switch (playerType) {
	case TYPE_TAGGER:
		break;
	case TYPE_PLAYER_YET:
		break;
	case TYPE_PLAYER:
		if (m_fCooltime >= VENT_OPEN_COOLTIME) {
#if USE_NETWORK
			Network& network = *Network::GetInstance();

			cs_packet_request_open_hidden_door packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_HIDDEN_DOOR;
			packet.door_num = Input::GetInstance()->m_pPlayer->m_vent_number;

			network.send_packet(&packet);
			SetOpen(true);
#endif
#if !USE_NETWORK
			SetOpen(true);
#endif
		}
		break;
	case TYPE_DEAD_PLAYER:
		break;
	}
}

void Vent::SetRotation(DIR d)
{
	switch (d)
	{
	case DEGREE0:
		m_dir = DEGREE0;
		break;
	case DEGREE90:
		m_dir = DEGREE90;
		Rotate(0, 90, 0);
		break;
	case DEGREE180:
		m_dir = DEGREE180;
		Rotate(0, 180, 0);
		break;
	default:
		m_dir = DEGREE270;
		Rotate(0, 270, 0);
		break;
	}
}

void Vent::update(float fElapsedTime)
{
	m_fCooltime += fElapsedTime;
	m_fGauge = m_fCooltime / VENT_OPEN_COOLTIME;
#if !USE_NETWORK
	if (IsOpen)
		if (m_fCooltime >= VENT_CLOSE_COOLTIME)
			SetOpen(false);
#endif
}

Door::Door() : InteractionObject()
{
	m_nUIs = 3;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	m_nUIType = DOOR_UI;

	LeftDoorPos = XMFLOAT3();
	RightDoorPos = XMFLOAT3();

	m_nSound = 1;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/Door.wav", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 20.0f, 50.0f);
}

Door::~Door()
{
	delete[] m_pSounds;
}

void Door::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);

	GameObject* leftDoor = FindFrame("Left_Door_Final");
	GameObject* rightDoor = FindFrame("Right_Door_Final");

	LeftDoorPos = leftDoor->GetPosition();
	RightDoorPos = rightDoor->GetPosition();

	m_fPitch = fPitch; m_fYaw = fYaw; m_fRoll = fRoll;

	IsRot = true;

}

bool Door::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 2.5f;
		maxz = m_xmf4x4ToParent._43 + 2.5f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 2.5f;
		maxx = m_xmf4x4ToParent._41 + 2.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 2.5f;
		maxz = m_xmf4x4ToParent._43 + 2.5f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 2.5f;
		maxx = m_xmf4x4ToParent._41 + 2.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	}
	if (PlayerPos.x > maxx) {
		IsNear = false;
		return false; 
	}
	if (PlayerPos.x < minx) {
		IsNear = false;
		return false; 
	}
	if (PlayerPos.z < minz) {
		IsNear = false;
		return false; 
	}
	if (PlayerPos.z > maxz) {
		IsNear = false;
		return false; 
	}
	IsNear = true;
	return true;
}

void Door::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	GameObject* LeftDoor = FindFrame("Left_Door_Final");
	GameObject* rightDoor = FindFrame("Right_Door_Final");

	float prevLeftX = LeftDoor->m_xmf4x4ToParent._41;
	float newLeftX = prevLeftX + OpenTime;
	LeftDoor->m_xmf4x4ToParent._41 = newLeftX;

	float prevRightX = rightDoor->m_xmf4x4ToParent._41;
	float newRightX = prevRightX - OpenTime;
	rightDoor->m_xmf4x4ToParent._41 = newRightX;

	UpdateTransform(nullptr);

	GameObject::render(pd3dCommandList);

	LeftDoor->m_xmf4x4ToParent._41 = prevLeftX;
	rightDoor->m_xmf4x4ToParent._41 = prevRightX;

}

void Door::update(float fElapsedTime)
{
	if (IsWorking == true && m_bIsBlocked);
	else if (m_bIsBlocked) {
		m_fCooltime = 0;
		return;
	}
	if (IsInteraction) {
		if (IsNear) {
			if (!IsWorking)
				m_fCooltime += fElapsedTime;
			else
				m_fCooltime = 0;
			UCHAR keyBuffer[256];
			memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
			if (((keyBuffer['f'] & 0xF0) == false) && ((keyBuffer['F'] & 0xF0) == false)) {
				m_fCooltime = 0;
				IsInteraction = false;
			}
		}
		else {
			IsInteraction = false;
			m_fCooltime = 0;
		}
	}
	else {
		m_fCooltime = 0;
	}
	if (IsOpen) {
		OpenTime += fElapsedTime;
		if (OpenTime >= 1.6f) {
			OpenTime = 1.6f;
			IsWorking = false;
		}
	}
	else {
		OpenTime -= fElapsedTime;
		if (OpenTime <= 0.0f) {
			OpenTime = 0.0f;
			IsWorking = false;
		}
	}
	if (Input::GetInstance()->m_pPlayer->m_Type == TYPE_TAGGER) {
		if (IsOpen)
			m_fGauge = m_fCooltime / DOOR_CLOSE_COOLTIME_TAGGER;
		else
			m_fGauge = m_fCooltime / DOOR_OPEN_COOLTIME_TAGGER;
	}
	else if (Input::GetInstance()->m_pPlayer->m_Type == TYPE_PLAYER) {
		if (IsOpen)
			m_fGauge = m_fCooltime / DOOR_CLOSE_COOLTIME_PLAYER;
		else
			m_fGauge = m_fCooltime / DOOR_OPEN_COOLTIME_PLAYER;
	}
	else if (Input::GetInstance()->m_pPlayer->m_Type == TYPE_PLAYER_YET) {
		if (IsOpen)
			m_fGauge = m_fCooltime / DOOR_CLOSE_COOLTIME_PLAYER;
		else
			m_fGauge = m_fCooltime / DOOR_OPEN_COOLTIME_PLAYER;
	}
	else {
		if (IsOpen)
			m_fGauge = m_fCooltime / DOOR_CLOSE_COOLTIME_DEAD_PLAYER;
		else
			m_fGauge = m_fCooltime / DOOR_OPEN_COOLTIME_DEAD_PLAYER;
	}
}

void Door::SetPosition(XMFLOAT3 xmf3Position)
{
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);

	GameObject* leftDoor = FindFrame("Left_Door_Final");
	GameObject* rightDoor = FindFrame("Right_Door_Final");

	LeftDoorPos = leftDoor->GetPosition();
	RightDoorPos = rightDoor->GetPosition();

	Sound& sound = *Sound::GetInstance();
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void Door::SetPosition(float x, float y, float z)
{
	GameObject::SetPosition(x, y, z);

	GameObject* leftDoor = FindFrame("Left_Door_Final");
	GameObject* rightDoor = FindFrame("Right_Door_Final");

	LeftDoorPos = leftDoor->GetPosition();
	RightDoorPos = rightDoor->GetPosition();

	Sound& sound = *Sound::GetInstance();
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void Door::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//int playerType = Input::GetInstance()->m_pPlayer->GetType();
	if (IsWorking) return;
	if (IsNear) {
		if (true == m_bIsBlocked) {
			if (m_ppInteractionUIs[2]) {
				m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
				m_ppInteractionUIs[2]->BillboardRender(pd3dCommandList, m_dir, 1.0f, m_nUIType);
			}
		}
		else {
			if (true == IsOpen) {
				if (m_ppInteractionUIs[1]) {
					m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
					m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
			else {
				if (m_ppInteractionUIs[0]) {
					m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
					m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
		}
	}
}

void Door::Interaction(int playerType)
{
	if (false == IsInteraction) {
		if (IsWorking) return;
		if (m_fCooltime > 0) m_fCooltime = 0;
		IsInteraction = true;
	}
	if (IsOpen) {
		switch (playerType) {
		case TYPE_TAGGER:
			if (m_fCooltime >= DOOR_CLOSE_COOLTIME_TAGGER) {
				SetOpen(false);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		case TYPE_PLAYER_YET:
		case TYPE_PLAYER:
			if (m_fCooltime >= DOOR_CLOSE_COOLTIME_PLAYER) {
				SetOpen(false);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		case TYPE_DEAD_PLAYER:
			if (m_fCooltime >= DOOR_CLOSE_COOLTIME_DEAD_PLAYER) {
				SetOpen(false);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		}
	}
	else {
		switch (playerType) {
		case TYPE_TAGGER:
			if (m_fCooltime >= DOOR_OPEN_COOLTIME_TAGGER) {
				SetOpen(true);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		case TYPE_PLAYER_YET:
		case TYPE_PLAYER:
			if (m_fCooltime >= DOOR_OPEN_COOLTIME_PLAYER) {
				SetOpen(true);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		case TYPE_DEAD_PLAYER:
			if (m_fCooltime >= DOOR_OPEN_COOLTIME_DEAD_PLAYER) {
				SetOpen(true);
				cs_packet_request_open_door packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR;
				packet.door_num = Input::GetInstance()->m_pPlayer->m_door_number;
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.send_packet(&packet);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
			break;
		}
	}
}

void Door::SetOpen(bool Open)
{
	if (IsWorking)
		return;
	if (IsOpen) {
		if (Open == false) {
			IsOpen = false;
			IsWorking = true;
			Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
		}
	}
	else {
		if (Open == true) {
			IsOpen = true;
			IsWorking = true;
			Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
		}
	}
}

void Door::SetRotation(DIR d)
{
	switch (d)
	{
	case DEGREE0:
		m_dir = DEGREE0;
		break;
	case DEGREE90:
		m_dir = DEGREE90;
		Rotate(0, 90, 0);
		break;
	case DEGREE180:
		m_dir = DEGREE180;
		Rotate(0, 180, 0);
		break;
	default:
		m_dir = DEGREE270;
		Rotate(0, 270, 0);
		break;
	}
}

bool Door::GetIsWorking()
{
	return IsWorking;
}

UIObject::UIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x, float y, float width, float height)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	UIMesh* pUIMesh = new UIMesh(pd3dDevice, pd3dCommandList,x,y,width,height);
	SetMesh(pUIMesh);

	Texture* pUITexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pUITexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pUITexture, 0, 17);

	Material* pUIMaterial = new Material(1);
	pUIMaterial->SetTexture(pUITexture);
	pUIMaterial->SetUIShader();

	renderer->SetMaterial(0, pUIMaterial);
}

UIObject::~UIObject()
{
}

InteractionObject::InteractionObject() : GameObject()
{
}

InteractionObject::~InteractionObject()
{
}

void InteractionObject::SetUI(int index, InteractionUI* ui)
{
	if (m_ppInteractionUIs[index]) m_ppInteractionUIs[index]->Release();
	m_ppInteractionUIs[index] = ui;
	if (m_ppInteractionUIs[index]) m_ppInteractionUIs[index]->AddRef();
}

int InteractionObject::GetDIR() const
{
	switch(m_dir) {
		case DEGREE0:
			return 0;
		case DEGREE90:
			return 1;
		case DEGREE180:
			return 2;
		default:
			return 3;
	}
}

InteractionUI::InteractionUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	Mesh* pUIMesh = new TexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 0.0f, 0.2f, 0.2f);
	SetMesh(pUIMesh);

	Texture* pUITexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pUITexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pUITexture, 0, 17);

	Material* pUIMaterial = new Material(1);
	pUIMaterial->SetTexture(pUITexture);
	pUIMaterial->SetDoorUIShader();

	renderer->SetMaterial(0, pUIMaterial);

	SetScale(0.2f, 0.2f, 0.2f);
	UpdateTransform(nullptr);
}

InteractionUI::~InteractionUI()
{
}

void InteractionUI::BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList, DIR d, float gauge, int type)
{
	UpdateShaderVariable(pd3dCommandList, gauge, type);
	switch (d)
	{
	case DEGREE0:
		//render(pd3dCommandList);
		break;
	case DEGREE90:
		Rotate(0, 90, 0);
		//render(pd3dCommandList);
		//Rotate(0, -90, 0);
		break;
	case DEGREE180:
		Rotate(0, 180, 0);
		//render(pd3dCommandList);
		//Rotate(0, -180, 0);
		break;
	default:
		Rotate(0, -90, 0);
		//render(pd3dCommandList);
		//Rotate(0, 90, 0);
		break;
	}
	XMFLOAT3 xmf3CameraPosition = Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition();

	SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));

	switch (d)
	{
	case DEGREE0:
		render(pd3dCommandList);
		break;
	case DEGREE90:
		//Rotate(0, 90, 0);
		render(pd3dCommandList);
		Rotate(0, -90, 0);
		break;
	case DEGREE180:
		//Rotate(0, 180, 0);
		render(pd3dCommandList);
		Rotate(0, -180, 0);
		break;
	default:
		//Rotate(0, -90, 0);
		render(pd3dCommandList);
		Rotate(0, 90, 0);
		break;
	}
}

void InteractionUI::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void InteractionUI::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type)
{
	float x = gauge;
	int t = type;
	pd3dCommandList->SetGraphicsRoot32BitConstants(18, 1, &x, 0);
	pd3dCommandList->SetGraphicsRoot32BitConstants(18, 1, &t, 1);
}

PowerSwitch::PowerSwitch() : InteractionObject()
{
	m_nUIs = 5;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];	
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	IsOpen = false;
	m_nUIType = POWER_UI;

	m_nSound = 3;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/Switch_2_On.wav", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f);
	m_pSounds[1] = Sound::GetInstance()->CreateObjectSound("Sound/Lever2.mp3", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f, true);
	m_pSounds[2] = Sound::GetInstance()->CreateObjectSound("Sound/Lever3.mp3", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f);
}

PowerSwitch::~PowerSwitch()
{
	delete[] m_pSounds;
}

void PowerSwitch::Init()
{
	GameObject* pKnob = FindFrame("Knob");

	m_pCup = FindFrame("Cup");
	m_pMainKnob = FindFrame("Main_Knob");
	m_xmf4x4MainKnobParent = m_pMainKnob->m_xmf4x4ToParent;
	for (int i = 0; i < 10; ++i) {
		SetAnswer(i, false);
	}
	SetAnswer(5, true);
}

bool PowerSwitch::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 0.5f;
		maxx = m_xmf4x4ToParent._41 + 0.5f;
		minz = m_xmf4x4ToParent._43 + 1.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 - 1.0f;
		minz = m_xmf4x4ToParent._43 - 0.5f;
		maxz = m_xmf4x4ToParent._43 + 0.5f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 0.5f;
		maxx = m_xmf4x4ToParent._41 + 0.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 - 1.0f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 - 1.0f;
		minz = m_xmf4x4ToParent._43 - 0.5f;
		maxz = m_xmf4x4ToParent._43 + 0.5f;
		break;
	}
	if (PlayerPos.x > maxx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.x < minx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z < minz) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z > maxz) {
		IsNear = false;
		return false;
	}
	IsNear = true;
	return true;
}

void PowerSwitch::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);	
	
	m_fPitch = fPitch; m_fYaw = fYaw; m_fRoll = fRoll;

	IsRot = true;
}

void PowerSwitch::SetOpen(bool Open)
{
	IsOpen = Open;
	m_fCooltime = 0;
}

void PowerSwitch::SetActivate(bool value)
{
	m_bClear = value;
}

void PowerSwitch::update(float fElapsedTime)
{
	if (m_bClear) return;
	m_fCooltime += fElapsedTime;
	if (m_bIsOperating == false) {
		if (m_bDoesOtherPlayerActive) m_fCheckCooltime += fElapsedTime;
		else {
			m_fCheckCooltime = 0;
		}
		return;
	}
	if (IsNear == false) {
		m_bIsOperating = false;
		m_fCooltime = 0;
		Input::GetInstance()->m_gamestate->ChangeSameLevelState();
		return;
	}
	if (false == IsOpen) return;
	UCHAR keyBuffer[256];
	memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
	if (keyBuffer['1'] & 0xF0) OperateKnob(0);
	if (keyBuffer['2'] & 0xF0) OperateKnob(1);
	if (keyBuffer['3'] & 0xF0) OperateKnob(2);
	if (keyBuffer['4'] & 0xF0) OperateKnob(3);
	if (keyBuffer['5'] & 0xF0) OperateKnob(4);
	if (keyBuffer['6'] & 0xF0) OperateKnob(5);
	if (keyBuffer['7'] & 0xF0) OperateKnob(6);
	if (keyBuffer['8'] & 0xF0) OperateKnob(7);
	if (keyBuffer['9'] & 0xF0) OperateKnob(8);
	if (keyBuffer['0'] & 0xF0) OperateKnob(9);

	if (keyBuffer['c'] & 0xF0 || keyBuffer['C'] & 0xF0) {
		Reset();
		m_bIsOperating = false;
		m_fCooltime = 0;
		Input::GetInstance()->m_gamestate->ChangeSameLevelState();
	}
	if (keyBuffer['f'] & 0xF0 || keyBuffer['F'] & 0xF0) {
		if (m_fCooltime < GLOBAL_INTERACTION_COOLTIME) return;
		if (IsEqual(m_fCheckCooltime, 0)) {
#if USE_NETWORK
			Network& network = *Network::GetInstance();

			cs_packet_electronic_system_lever_working packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_ELETRONIC_SYSTEM_LEVER_WORKING;
			packet.index = m_switch_index;
			packet.is_start = true;

			network.send_packet(&packet);
#endif
			Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 1.0f);
		}
		m_fCheckCooltime += fElapsedTime;
		if (m_fCheckCooltime < 2.0f) return;

		if (false == CheckAnswer()) {
			Reset();
			m_bIsOperating = false;
			m_fCheckCooltime = 0;
			Input::GetInstance()->m_gamestate->ChangeSameLevelState();
			// 다른 플레이어들에게 CheckStop();
#if USE_NETWORK
			Network& network = *Network::GetInstance();

			cs_packet_electronic_system_lever_working packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_ELETRONIC_SYSTEM_LEVER_WORKING;
			packet.index = m_switch_index;
			packet.is_start = false;

			network.send_packet(&packet);
#endif
			Sound::GetInstance()->StopObjectSound(m_pSounds[1]);
			Sound::GetInstance()->PlayObjectSound(m_pSounds[2], 1.0f);
			Input::GetInstance()->m_pPlayer->SetInfo(FIXFAIL);
			return;
		}

		m_bClear = true;
		m_bIsOperating = false;
		Input::GetInstance()->m_gamestate->ChangeSameLevelState();
		Input::GetInstance()->m_pPlayer->UseItem();
#if USE_NETWORK
		cs_packet_request_electronic_system_activate packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_ATIVATE;
		packet.system_index = m_switch_index;
		Network& network = *Network::GetInstance();
		network.send_packet(&packet);
#endif
		Sound::GetInstance()->StopObjectSound(m_pSounds[1]);
		Sound::GetInstance()->PlayObjectSound(m_pSounds[2], 1.0f);
		Input::GetInstance()->m_pPlayer->SetInfo(FIXSUCCESS);
	}
	else {
		if (false == IsEqual(m_fCheckCooltime, 0)) {
			// 다른 플레이어들에게 CheckStop();
#if USE_NETWORK
			Network& network = *Network::GetInstance();

			cs_packet_electronic_system_lever_working packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_ELETRONIC_SYSTEM_LEVER_WORKING;
			packet.index = m_switch_index;
			packet.is_start = false;

			network.send_packet(&packet);
#endif
			Sound::GetInstance()->StopObjectSound(m_pSounds[1]);
			Sound::GetInstance()->PlayObjectSound(m_pSounds[2], 1.0f);
		}
		m_fCheckCooltime = 0;
	}
}

void PowerSwitch::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (IsOpen) {
		m_pCup->isNotDraw = true;
		if (m_pCup->m_pChild) m_pCup->m_pChild->isNotDraw = true;
	}
	else {
		m_pCup->isNotDraw = false;
		if (m_pCup->m_pChild) m_pCup->m_pChild->isNotDraw = false;
	}
	for (int i = 0; i < 10; ++i) {
		std::string str = "Knob";
		str += "00" + std::to_string(i);
		GameObject* pKnob = FindFrame(str.c_str());
		if (pKnob) {
			m_fOffKnobPos = pKnob->GetPosition().x;		
			if (true == m_bOnAndOff[i]) {
				m_fOnKnobPos = m_fOffKnobPos;
				m_fOnKnobPos -= 0.2f;
				pKnob->m_xmf4x4ToParent._41 = -0.21f;
			}
			else {		
				pKnob->m_xmf4x4ToParent._41 = 0.07291567f;
			}
		}
	}
	XMFLOAT4X4 prevMat = m_pMainKnob->m_xmf4x4ToParent;
	UpdateTransform(nullptr);
	if (m_bIsOperating || m_bDoesOtherPlayerActive) {
		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fCheckCooltime * -35), XMConvertToRadians(0), XMConvertToRadians(0));
		m_pMainKnob->m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_pMainKnob->m_xmf4x4ToParent);
		
		UpdateTransform(NULL);
	}
	else {
		m_pMainKnob->m_xmf4x4ToParent = m_xmf4x4MainKnobParent;
	}
	if (m_bClear) {
		FindFrame("Lamp_1")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		FindFrame("Lamp_2")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.3f, 0.1f, 0.1f, 1.0f);
		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(-110.0f), XMConvertToRadians(0), XMConvertToRadians(0));
		m_pMainKnob->m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4MainKnobParent);
		UpdateTransform(NULL);
	}
	else {
		FindFrame("Lamp_1")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.1f, 0.3f, 0.1f, 1.0f);
		FindFrame("Lamp_2")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	GameObject::render(pd3dCommandList);
	m_pMainKnob->m_xmf4x4ToParent = prevMat;
}

void PowerSwitch::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int playerType = Input::GetInstance()->m_pPlayer->GetType();
	if (m_bClear) return;
	if (IsNear) {
		if (false == IsOpen) {
			if (playerType == TYPE_PLAYER) {
				if (m_ppInteractionUIs[0]) {
					if (m_dir == DEGREE0) {
						m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 - 0.5f);
					}
					else if (m_dir == DEGREE180) {
						m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 + 0.5f);
					}
					else {
						m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.5f, m_xmf4x4ToParent._43);
					}
					m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
		}
		else {
			if (playerType == TYPE_TAGGER) {
				if (m_ppInteractionUIs[1]) {
					if (m_dir == DEGREE0) {
						m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 - 0.5f);
					}
					else if (m_dir == DEGREE180) {
						m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 + 0.5f);
					}
					else {
						m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.5f, m_xmf4x4ToParent._43);
					}
					m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
				return;
			}
			if (m_bIsOperating) {
				if (m_ppInteractionUIs[3]) {
					if (m_dir == DEGREE0) {
						m_ppInteractionUIs[3]->SetPosition(m_xmf4x4ToParent._41 - 1.0f, 1.5f, m_xmf4x4ToParent._43 - 0.5f);
					}
					else if (m_dir == DEGREE180) {
						m_ppInteractionUIs[3]->SetPosition(m_xmf4x4ToParent._41 + 1.0f, 1.5f, m_xmf4x4ToParent._43 + 0.5f);
					}
					else {
						m_ppInteractionUIs[3]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.5f, m_xmf4x4ToParent._43 - 1.0f);
					}
					m_ppInteractionUIs[3]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
				if (m_ppInteractionUIs[2]) {
					if (m_dir == DEGREE0) {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41 - 1.0f, 1.0f, m_xmf4x4ToParent._43 - 0.5f);
					}
					else if (m_dir == DEGREE180) {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41 + 1.0f, 1.0f, m_xmf4x4ToParent._43 + 0.5f);
					}
					else {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.0f, m_xmf4x4ToParent._43 - 1.0f);
					}
					m_ppInteractionUIs[2]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
			else {
				if (m_ppInteractionUIs[2]) {
					if (m_dir == DEGREE0) {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 - 0.5f);
					}
					else if (m_dir == DEGREE180) {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 + 0.5f);
					}
					else {
						m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.5f, m_xmf4x4ToParent._43);
					}
					m_ppInteractionUIs[2]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
		}
	}
}

void PowerSwitch::Interaction(int playerType)
{
	if (m_bClear) return;
	if (IsOpen) {
		switch (playerType) {
		case TYPE_TAGGER:
		{
			Reset();
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			cs_packet_request_electronic_system_open packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_DOOR;
			packet.es_num = m_switch_index;
			packet.is_door_open = false;
			network.send_packet(&packet);


			cs_packet_request_electronic_system_reset update_packet;
			update_packet.size = sizeof(update_packet);
			update_packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_TAGGER;
			update_packet.switch_index = m_switch_index;
			network.send_packet(&update_packet);
#endif
			SetOpen(false);
			break;
		}

		case TYPE_PLAYER_YET:
		{
			break;
		}

		case TYPE_PLAYER:
		{
			if (m_bIsOperating) break;
			if (m_fCooltime >= GLOBAL_INTERACTION_COOLTIME) {
				m_fCooltime = 0;
				m_bIsOperating = true;
				Input::GetInstance()->m_pPlayer->SetLookAt(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43));
				Input::GetInstance()->m_gamestate->ChangeSameLevelState();
			}
			break;
		}
		case TYPE_DEAD_PLAYER:
		{
			break;
		}

		}
	}
	else {
		switch (playerType) {
		case TYPE_TAGGER:
		case TYPE_PLAYER_YET:
		case TYPE_DEAD_PLAYER:
			break;
		case TYPE_PLAYER:
		{
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			cs_packet_request_electronic_system_open packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_DOOR;
			packet.es_num = m_switch_index;
			packet.is_door_open = true;
			network.send_packet(&packet);
#endif
			SetOpen(true);
			break;
		}

		}
	}
}

void PowerSwitch::PowerOperate()
{
}

void PowerSwitch::SetIndex(int index)
{
	m_switch_index = index;
}

void PowerSwitch::SetRotation(DIR d)
{
	switch (d)
	{
	case DEGREE0:
		m_dir = DEGREE0;
		UpdateTransform(NULL);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		break;
	case DEGREE90:
		m_dir = DEGREE90;
		Rotate(0, 90, 0); 
		m_xmf3Right = XMFLOAT3(0.0f, 0.0f, -1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(1.0f, 0.0f, 0.0f);
		break;
	case DEGREE180:
		m_dir = DEGREE180;
		Rotate(0, 180, 0);
		m_xmf3Right = XMFLOAT3(-1.0f, 0.0f, 0.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, -1.0f);
		break;
	default:
		m_dir = DEGREE270;
		Rotate(0, 270, 0);
		m_xmf3Right = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(1.0f, 0.0f, 0.0f);
		break;
	}
}

void PowerSwitch::SetSwitchValue(int index, bool value)
{
	m_bOnAndOff[index] = value;
}

void PowerSwitch::SetAnswer(int index, bool answer)
{
	m_bAnswers[index] = answer;
}

void PowerSwitch::OperateKnob(int index)
{
	if (m_fCooltime < KNOB_OPERATE_COOLTIME) return;
	m_bOnAndOff[index] = !m_bOnAndOff[index];
	m_fCooltime = 0;
	Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
#if USE_NETWORK
	Network& network = *Network::GetInstance();
	cs_packet_request_eletronic_system_switch_control packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_SWICH;
	packet.electronic_system_index = m_switch_index;
	packet.switch_idx = index;
	packet.switch_value = m_bOnAndOff[index];

	network.send_packet(&packet);
#endif
}

bool PowerSwitch::CheckAnswer()
{
	for (int i = 0; i < 10; ++i) {
		if (m_bAnswers[i] != m_bOnAndOff[i])
			return false;
	}
	return true;
}

void PowerSwitch::Reset()
{
	for (int i = 0; i < 10; ++i) {
		m_bOnAndOff[i] = false;
	}
	m_fCooltime = 0;
#if USE_NETWORK
	Network& network = *Network::GetInstance();
	cs_packet_request_electronic_system_reset update_packet;
	update_packet.size = sizeof(update_packet);
	update_packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_RESET_BY_PLAYER;
	update_packet.switch_index = m_switch_index;
	network.send_packet(&update_packet);
#endif
}

void PowerSwitch::SetPosition(XMFLOAT3 xmf3Position)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void PowerSwitch::SetPosition(float x, float y, float z)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(x, y, z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void PowerSwitch::CheckStart()
{
	m_bDoesOtherPlayerActive = true;
	Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 0.9f);
}

void PowerSwitch::CheckStop()
{
	m_bDoesOtherPlayerActive = false;
	Sound::GetInstance()->StopObjectSound(m_pSounds[1]);
	Sound::GetInstance()->PlayObjectSound(m_pSounds[2], 1.0f);
}

Item::Item()
{
}

Item::~Item()
{
}

bool Item::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	return false;
}

void Item::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void Item::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	
}

void Item::Interaction(int playerType)
{
	switch (playerType) {
	case TYPE_TAGGER:
		break;
	case TYPE_PLAYER_YET:
		break;
	case TYPE_PLAYER:
		break;
	case TYPE_DEAD_PLAYER:
		break;
	}
}

ItemBox::ItemBox() : InteractionObject()
{
	m_nUIs = 4;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	m_nUIType = BOX_UI;
	m_dir = DEGREE0;
	for (int i = 0; i < 6; ++i) {
		m_pItems[i] = nullptr;
	}

	m_nSound = 1;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/car_door.wav", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f);
}

ItemBox::~ItemBox()
{
	delete[] m_pSounds;
}

bool ItemBox::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx{}, maxx{}, minz{}, maxz{};
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 2.2f;
		maxx = m_xmf4x4ToParent._41 + 2.2f;
		minz = m_xmf4x4ToParent._43 - 0.5f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 0.5f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 2.2f;
		maxz = m_xmf4x4ToParent._43 + 2.2f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 2.2f;
		maxx = m_xmf4x4ToParent._41 + 2.2f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 0.5f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 0.5f;
		minz = m_xmf4x4ToParent._43 - 2.2f;
		maxz = m_xmf4x4ToParent._43 + 2.2f;
		break;
	}
	if (PlayerPos.x > maxx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.x < minx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z < minz) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z > maxz) {
		IsNear = false;
		return false;
	}
	IsNear = true;
	return true;
}

void ItemBox::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (false == IsOpen)
	{
		GameObject* cap = FindFrame("Object005");
		cap->m_xmf4x4ToParent = m_xmf4x4CapMatrix;
		UpdateTransform(nullptr);
		GameObject::render(pd3dCommandList);
		return;
	}
	GameObject* cap = FindFrame("Object005");
	cap->m_xmf4x4ToParent = m_xmf4x4CapOpenMatrix;
	UpdateTransform(nullptr);
	GameObject::render(pd3dCommandList);
	if (m_bShownItem) {
		// Item Render
		switch (m_item) {
		case GAME_ITEM::ITEM_HAMMER:
			if (m_pItems[0]) {
				m_pItems[0]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[0]->render(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_DRILL:
			if (m_pItems[1]) {
				m_pItems[1]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[1]->render(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_WRENCH:
			if (m_pItems[2]) {
				m_pItems[2]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[2]->render(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_PLIERS:
			if (m_pItems[3]) {
				m_pItems[3]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[3]->render(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_DRIVER:
			if (m_pItems[4]) {
				m_pItems[4]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[4]->render(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_LIFECHIP:
			if (m_pItems[5]) {
				m_pItems[5]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[5]->render(pd3dCommandList);
			}
			break;
		default:
			// NONE
			break;
		}
	}
}

void ItemBox::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int playerType = Input::GetInstance()->m_pPlayer->GetType();
	if (IsNear) {
		if (true == IsOpen) {
			if (playerType == TYPE_PLAYER || playerType == TYPE_DEAD_PLAYER) {
				if (m_item == GAME_ITEM::ITEM_NONE) return;
				if (m_ppInteractionUIs[2]) {
					m_ppInteractionUIs[2]->SetPosition(m_xmf4x4ToParent._41, 0.5f, m_xmf4x4ToParent._43);
					m_ppInteractionUIs[2]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
				}
			}
			else if (playerType == TYPE_TAGGER) {
				if (m_ppInteractionUIs[1]) {
					m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 0.5f, m_xmf4x4ToParent._43 + 0.5f);
					m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
		}
		else {
			if (playerType == TYPE_PLAYER || playerType == TYPE_DEAD_PLAYER) {
				if (m_ppInteractionUIs[0]) {
					m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 0.5f, m_xmf4x4ToParent._43 + 0.5f);
					m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
				}
			}
		}
	}
}

void ItemBox::update(float fElapsedTime)
{
	if (IsOpen) {
		m_fPickupCooltime += fElapsedTime;
		if (IsInteraction) {
			if (IsNear) {
				m_fCooltime += fElapsedTime;

				UCHAR keyBuffer[256];
				memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
				if (((keyBuffer['f'] & 0xF0) == false) && ((keyBuffer['F'] & 0xF0) == false)) {
					m_fCooltime = 0;
					IsInteraction = false;
				}
			}
			else {
				IsInteraction = false;
				m_fCooltime = 0;
			}
		}
		else {
			m_fCooltime = 0;
		}
	}
	else {
		if (IsInteraction) {
			if (IsNear) {
				m_fCooltime += fElapsedTime;

				UCHAR keyBuffer[256];
				memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
				if (((keyBuffer['f'] & 0xF0) == false) && ((keyBuffer['F'] & 0xF0) == false)) {
					m_fCooltime = 0;
					IsInteraction = false;
				}
			}
			else {
				IsInteraction = false;
				m_fCooltime = 0;
			}
		}
		else {
			m_fCooltime = 0;
		}
	}
	m_fGauge = m_fCooltime / BOX_OPEN_COOLTIME;
}

void ItemBox::Depthrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (false == IsOpen)
	{
		GameObject* cap = FindFrame("Object005");
		cap->m_xmf4x4ToParent = m_xmf4x4CapMatrix;
		UpdateTransform(nullptr);
		GameObject::Depthrender(pd3dCommandList);
		return;
	}
	GameObject* cap = FindFrame("Object005");
	cap->m_xmf4x4ToParent = m_xmf4x4CapOpenMatrix;
	UpdateTransform(nullptr);
	GameObject::Depthrender(pd3dCommandList);
	if (m_bShownItem) {
		// Item Render
		switch (m_item) {
		case GAME_ITEM::ITEM_HAMMER:
			if (m_pItems[0]) {
				m_pItems[0]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[0]->Depthrender(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_DRILL:
			if (m_pItems[1]) {
				m_pItems[1]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[1]->Depthrender(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_WRENCH:
			if (m_pItems[2]) {
				m_pItems[2]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[2]->Depthrender(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_PLIERS:
			if (m_pItems[3]) {
				m_pItems[3]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[3]->Depthrender(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_DRIVER:
			if (m_pItems[4]) {
				m_pItems[4]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[4]->Depthrender(pd3dCommandList);
			}
			break;
		case GAME_ITEM::ITEM_LIFECHIP:
			if (m_pItems[5]) {
				m_pItems[5]->SetPosition(m_xmf4x4ToParent._41, 0, m_xmf4x4ToParent._43);
				m_pItems[5]->Depthrender(pd3dCommandList);
			}
			break;
		default:
			// NONE
			break;
		}
	}
}

void ItemBox::Interaction(int playerType)
{
	IsInteraction = true;
	if (IsOpen) {
		switch (playerType) {
		case TYPE_TAGGER: 
		{
			if (m_fCooltime >= BOX_CLOSE_COOLTIME) {
#if !USE_NETWORK
				SetOpen(false);
#endif
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.Send_Fix_Object_Box_Update(m_item_box_index, false);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
			}
		}
		break;
		case TYPE_PLAYER_YET:
			IsInteraction = false;
			break;
		case TYPE_DEAD_PLAYER:
		case TYPE_PLAYER:
			IsInteraction = false;
			if (m_item == GAME_ITEM::ITEM_NONE) break;
			if (m_fPickupCooltime >= GLOBAL_INTERACTION_COOLTIME) {
				if (Input::GetInstance()->m_pPlayer->PickUpItem(m_item)) {
#if USE_NETWORK
					Network& network = *Network::GetInstance();
					network.Send_Picking_Fix_Object_Packet(m_item_box_index, m_item);
					//std::cout << "m_item_box_index : " << m_item_box_index << " -> ";
					//if (m_item == GAME_ITEM::ITEM_DRILL)
					//	std::cout << "ITEM_DRILL pick" << std::endl;
					//else if (m_item == GAME_ITEM::ITEM_DRIVER)
					//	std::cout << "ITEM_DRIVER pick" << std::endl;
					//else if (m_item == GAME_ITEM::ITEM_HAMMER)
					//	std::cout << "ITEM_HAMMER pick" << std::endl;
					//else if (m_item == GAME_ITEM::ITEM_LIFECHIP)
					//	std::cout << "ITEM_LIFECHIP pick" << std::endl;
					//else if (m_item == GAME_ITEM::ITEM_PLIERS)
					//	std::cout << "ITEM_PLIERS pick" << std::endl;
					//else if (m_item == GAME_ITEM::ITEM_WRENCH)
					//	std::cout << "ITEM_WRENCH pick" << std::endl;
#endif
					m_item = GAME_ITEM::ITEM_NONE;
					m_fPickupCooltime = 0;
				}
			}
			break;
		}
	}
	else {
		switch (playerType) {
		case TYPE_TAGGER:
		case TYPE_PLAYER_YET:
			IsInteraction = false;
			break;
		case TYPE_DEAD_PLAYER:
			if (m_fCooltime >= BOX_OPEN_DEAD_COOLTIME) {
#if !USE_NETWORK
				SetOpen(true);
#endif
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.Send_Fix_Object_Box_Update(m_item_box_index, true);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
				m_fPickupCooltime = 0;
			}
			break;
		case TYPE_PLAYER:
			if (m_fCooltime >= BOX_OPEN_COOLTIME) {
#if !USE_NETWORK
				SetOpen(true);
#endif
#if USE_NETWORK
				Network& network = *Network::GetInstance();
				network.Send_Fix_Object_Box_Update(m_item_box_index, true);
#endif
				m_fCooltime = 0;
				IsInteraction = false;
				m_fPickupCooltime = 0;
			}
			break;
		}
	}
}

void ItemBox::SetOpen(bool open)
{
	if (false == open) {
		if (false == IsOpen) return;
		IsOpen = false;
		Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 0.5f);
	}
	else {
		if (true == IsOpen) return;
		IsOpen = true;
		Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 0.5f);
	}
}

void ItemBox::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void ItemBox::SetItem(GAME_ITEM::ITEM item)
{
	m_bShownItem = true;
	m_item = item;
}

void ItemBox::InitItems(int index, GameObject* item)
{
	if (m_pItems[index]) m_pItems[index]->Release();
	if (item) m_pItems[index] = item;
	if (m_pItems[index]) m_pItems[index]->AddRef();
}

void ItemBox::SetPosition(XMFLOAT3 xmf3Position)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void ItemBox::SetPosition(float x, float y, float z)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(x, y, z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void ItemBox::SetRotation(DIR d)
{
	GameObject* cap = FindFrame("Object005");
	m_xmf4x4CapOpenMatrix = m_xmf4x4CapMatrix = cap->m_xmf4x4ToParent;
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90), XMConvertToRadians(0), XMConvertToRadians(0));
	m_xmf4x4CapOpenMatrix = Matrix4x4::Multiply(mtxRotate, m_xmf4x4CapOpenMatrix);
	switch (d)
	{
	case DEGREE0:
		m_dir = DEGREE0;
		break;
	case DEGREE90:
		m_dir = DEGREE90;
		Rotate(0, 90, 0);
		break;
	case DEGREE180:
		m_dir = DEGREE180;
		Rotate(0, 180, 0);
		break;
	default:
		m_dir = DEGREE270;
		Rotate(0, -90, 0);
		break;
	}
}

IngameUI::IngameUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x, float y, float width, float height)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	UIMesh* pUIMesh = new UIMesh(pd3dDevice, pd3dCommandList, x, y, width, height);
	SetMesh(pUIMesh);

	Texture* pUITexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pUITexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pUITexture, 0, 17);

	Material* pUIMaterial = new Material(1);
	pUIMaterial->SetTexture(pUITexture);
	pUIMaterial->SetDoorUIShader();

	renderer->SetMaterial(0, pUIMaterial);
}

IngameUI::~IngameUI()
{
}

void IngameUI::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type)
{
	float x = gauge;
	int t = type;
	pd3dCommandList->SetGraphicsRoot32BitConstants(18, 1, &x, 0);
	pd3dCommandList->SetGraphicsRoot32BitConstants(18, 1, &t, 1);
}

void IngameUI::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UIrender(pd3dCommandList, m_fGauge, m_UIType);
}

void IngameUI::UIrender(ID3D12GraphicsCommandList* pd3dCommandList, float gauge, int type)
{
	UpdateShaderVariable(pd3dCommandList, gauge, type);
	GameObject::render(pd3dCommandList);
}

void IngameUI::SetAnswer(bool on)
{
	if (true == on) {
		m_xmf4x4World._41 = 0.15f;
	}
	else {
		m_xmf4x4World._41 = 0.0f;
	}
}

MinimapUI::MinimapUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName, float x, float y, float width, float height)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	UIMesh* pUIMesh = new UIMesh(pd3dDevice, pd3dCommandList, x, y, width, height);
	SetMesh(pUIMesh);

	Texture* pUITexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pUITexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pUITexture, 0, 17);

	Material* pUIMaterial = new Material(1);
	pUIMaterial->SetTexture(pUITexture);
	pUIMaterial->SetMinimapShader();

	renderer->SetMaterial(0, pUIMaterial);
}

MinimapUI::~MinimapUI()
{
}

void MinimapUI::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void MinimapUI::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UpdateShaderVariable(pd3dCommandList);
	GameObject::render(pd3dCommandList);
}

TaggersBox::TaggersBox()
{
	m_nUIs = 3;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	m_nUIType = TAGGER_UI;

	m_nChips = 12;
	m_ppChips = new GameObject * [m_nChips];
	for (int i = 0; i < m_nChips; ++i) {
		m_ppChips[i] = nullptr;
	}

	m_nSound = 1;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/Door.wav", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 30.0f, 80.0f);
}

TaggersBox::~TaggersBox()
{
	delete[] m_pSounds;
}

bool TaggersBox::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float dist = (m_xmf4x4ToParent._41 - PlayerPos.x) * (m_xmf4x4ToParent._41 - PlayerPos.x) + (m_xmf4x4ToParent._43 - PlayerPos.z) * (m_xmf4x4ToParent._43 - PlayerPos.z);
	if (dist > 25.0f) { 
		IsNear = false;
		return false; }
	IsNear = true;
	return true;
}

void TaggersBox::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void TaggersBox::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	GameObject::render(pd3dCommandList);
}

void TaggersBox::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (Input::GetInstance()->m_pPlayer->GetType() != TYPE_TAGGER) return;
	if (IsNear) {
		XMFLOAT3 pos = Input::GetInstance()->m_pPlayer->GetPosition();
		XMFLOAT3 AltarToPlayer = XMFLOAT3(pos.x - m_xmf4x4ToParent._41, pos.y - m_xmf4x4ToParent._42, pos.z - m_xmf4x4ToParent._43);
		if (m_bActivate) {
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			//  (술래가 생명칩이 있는 경우 
			if (network.m_lifechip) {
				if (m_ppInteractionUIs[1]) {
					m_ppInteractionUIs[1]->SetPosition(AltarToPlayer.x * 0.8f + m_xmf4x4ToParent._41, 2.0f, AltarToPlayer.z * 0.8f + m_xmf4x4ToParent._43);
					m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
				}
			}
			else {
				if (m_ppInteractionUIs[2]) {
					m_ppInteractionUIs[2]->SetPosition(AltarToPlayer.x * 0.8f + m_xmf4x4ToParent._41, 2.0f, AltarToPlayer.z * 0.8f + m_xmf4x4ToParent._43);
					m_ppInteractionUIs[2]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
				}
			}
#else
			if (m_ppInteractionUIs[1]) {
				m_ppInteractionUIs[1]->SetPosition(AltarToPlayer.x * 0.8f + m_xmf4x4ToParent._41, 2.0f, AltarToPlayer.z * 0.8f + m_xmf4x4ToParent._43);
				m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
			}
#endif
		}
		else {
			if (m_ppInteractionUIs[0]) {
				m_ppInteractionUIs[0]->SetPosition(AltarToPlayer.x * 0.8f + m_xmf4x4ToParent._41, 2.0f, AltarToPlayer.z * 0.8f + m_xmf4x4ToParent._43);
				m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
			}
		}
	}
}

void TaggersBox::update(float fElapsedTime)
{
	if (m_nLifeChips > 11) {
		// Tagger's win
		printf("tagger's win");
	}
	if (false == m_bActivate) {
		if (IsInteraction) {
			if (IsNear) {
				m_fCooltime += fElapsedTime;

				UCHAR keyBuffer[256];
				memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
				if (((keyBuffer['f'] & 0xF0) == false) && ((keyBuffer['F'] & 0xF0) == false)) {
					m_fCooltime = 0;
					IsInteraction = false;
				}
			}
			else {
				IsInteraction = false;
				m_fCooltime = 0;
			}
		}
		else {
			m_fCooltime = 0;
		}
	}
	m_fGauge = m_fCooltime / TAGGER_ACTIVATION_COOLTIME;
}

void TaggersBox::Interaction(int playerType)
{
	if (playerType != TYPE_TAGGER) return;
	if (m_bActivate) { // 활성화 상태
#if USE_NETWORK
		// 생명칩 있을 때 넣는 동작
		Network& network = *Network::GetInstance();
		if (network.m_lifechip) {
			network.m_lifechip = false;
			network.Send_Altar_Event();
		}
#endif
	}
	else {
		IsInteraction = true;
		if (m_fCooltime >= TAGGER_ACTIVATION_COOLTIME) {
#if	USE_NETWORK
			Network& network = *Network::GetInstance();
			network.Send_Ativate_Altar();
#endif
			m_fCooltime = 0;
			m_bActivate = true;
			IsInteraction = false;
		}
	}
}

void TaggersBox::SetOpen(bool open)
{
}

void TaggersBox::SetRotation(DIR d)
{
}

void TaggersBox::SetPosition(XMFLOAT3 xmf3Position)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void TaggersBox::SetPosition(float x, float y, float z)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(x, y, z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void TaggersBox::CollectChip()
{
	m_ppChips[m_nLifeChips++]->SetDraw(false);
}

void TaggersBox::Reset()
{
	m_nLifeChips = 0;
	m_bActivate = false;
	Init();
}

void TaggersBox::Init()
{
	for (int i = 0; i < m_nChips; ++i) {
		std::string str = "Chip";
		str += std::to_string(i);
		m_ppChips[i] = FindFrame(str.c_str());
		m_ppChips[i]->SetDraw(true);
	}
}

EscapeObject::EscapeObject()
{
	m_nUIs = 2;
	m_ppInteractionUIs = new InteractionUI * [m_nUIs];
	for (int i = 0; i < m_nUIs; ++i) {
		m_ppInteractionUIs[i] = nullptr;
	}
	m_nUIType = DOOR_UI;

	m_nSound = 2;
	m_pSounds = new int[m_nSound];
	m_pSounds[0] = Sound::GetInstance()->CreateObjectSound("Sound/Lever2.mp3", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f, true);
	m_pSounds[1] = Sound::GetInstance()->CreateObjectSound("Sound/Lever3.mp3", m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43, 10.0f, 30.0f);
}

EscapeObject::~EscapeObject()
{
	delete[] m_pSounds;
}

void EscapeObject::Init()
{
	m_pArm = FindFrame("Arm");
	m_xmf4x4ArmParent = m_pArm->m_xmf4x4ToParent;
}

bool EscapeObject::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 0.7f;
		maxz = m_xmf4x4ToParent._43 + 0.7f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 0.7f;
		maxx = m_xmf4x4ToParent._41 + 0.3f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 0.7f;
		maxz = m_xmf4x4ToParent._43 + 0.7f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 2.5f;
		maxx = m_xmf4x4ToParent._41 + 2.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	}
	if (PlayerPos.x > maxx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.x < minx) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z < minz) {
		IsNear = false;
		return false;
	}
	if (PlayerPos.z > maxz) {
		IsNear = false;
		return false;
	}
	IsNear = true;
	return true;
}

void EscapeObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void EscapeObject::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 prevMat = m_pArm->m_xmf4x4ToParent;
	UpdateTransform(nullptr);
	if (IsInteraction || m_bDoesOtherPlayerActive) {
		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fCooltime * 9), XMConvertToRadians(0), XMConvertToRadians(0));
		m_pArm->m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_pArm->m_xmf4x4ToParent);

		UpdateTransform(nullptr);
	}
	else {
		m_pArm->m_xmf4x4ToParent = m_xmf4x4ArmParent;
	}

	GameObject::render(pd3dCommandList);
	m_pArm->m_xmf4x4ToParent = m_xmf4x4ArmParent;
}

void EscapeObject::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int playerType = Input::GetInstance()->m_pPlayer->GetType();
	if (IsNear == false) return;
	if (playerType == TYPE_TAGGER) return;
	if (IsWorking) {
		if (playerType == TYPE_PLAYER) {
			if (m_ppInteractionUIs[0]) {
				m_ppInteractionUIs[0]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43);
				m_ppInteractionUIs[0]->BillboardRender(pd3dCommandList, m_dir, m_fGauge * 0.8f, m_nUIType);
			}
		}
		else if (playerType == TYPE_DEAD_PLAYER) {
			if (m_ppInteractionUIs[1]) {
				m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43);
				m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
			}
		}
	}
	else {
		if (m_ppInteractionUIs[1]) {
			m_ppInteractionUIs[1]->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43);
			m_ppInteractionUIs[1]->BillboardRender(pd3dCommandList, m_dir, 0.0f, m_nUIType);
		}
	}
}

void EscapeObject::update(float fElapsedTime)
{
	if (false == IsWorking) return;
	if (IsInteraction == false) {
		if (m_bDoesOtherPlayerActive) m_fCooltime += fElapsedTime;
		else {
			m_fCooltime = 0;
		}
		return;
	}
	if (IsNear) {
		m_fCooltime += fElapsedTime;
		UCHAR keyBuffer[256];
		memcpy(keyBuffer, Input::GetInstance()->keyBuffer, (sizeof(keyBuffer)));
		if (((keyBuffer['f'] & 0xF0) == false) && ((keyBuffer['F'] & 0xF0) == false)) {
			if (false == IsEqual(m_fCooltime, 0)) {
#if USE_NETWORK
				// Send CheckStop();
				Network& network = *Network::GetInstance();

				cs_packet_request_escapesystem_lever_working packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_ESCAPESYSTEM_LEVER_WORKING;
				packet.index = GetID();
				packet.is_start = false;

				network.send_packet(&packet);
#endif
				Sound::GetInstance()->StopObjectSound(m_pSounds[0]);
				Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 1.0f);
			}
			m_fCooltime = 0;
			IsInteraction = false;
		}
	}
	else {
		if (false == IsEqual(m_fCooltime, 0)) {
#if USE_NETWORK
			// Send CheckStop();
			Network& network = *Network::GetInstance();

			cs_packet_request_escapesystem_lever_working packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_ESCAPESYSTEM_LEVER_WORKING;
			packet.index = GetID();
			packet.is_start = false;

			network.send_packet(&packet);
#endif
			Sound::GetInstance()->StopObjectSound(m_pSounds[0]);
			Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 1.0f);
		}
		m_fCooltime = 0;
		IsInteraction = false;
	}
	m_fGauge = m_fCooltime / PLAYER_ESCAPE_LEVER_COOLTIME;
}

void EscapeObject::Interaction(int playerType)
{
	if (false == IsWorking) return;
	if (playerType == TYPE_TAGGER || playerType == TYPE_PLAYER_YET) return;
	if (m_bDoesOtherPlayerActive) return;

	IsInteraction = true;
	if (IsEqual(m_fCooltime, 0)) {
#if USE_NETWORK
		//  Send CheckStart();
		Network& network = *Network::GetInstance();

		cs_packet_request_escapesystem_lever_working packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_ESCAPESYSTEM_LEVER_WORKING;
		packet.index = GetID();
		packet.is_start = true;

		network.send_packet(&packet);
#endif
		Sound::GetInstance()->PlayObjectSound(m_pSounds[0], 1.0f);
	}
	if (m_fCooltime >= PLAYER_ESCAPE_LEVER_COOLTIME) {
		if (m_bIsReal) {
#if USE_NETWORK
			Network& network = *Network::GetInstance();
			cs_packet_request_escapesystem_lever_working packet;
			packet.size = sizeof(packet);
			packet.type = CS_PACKET::CS_PACKET_REQUEST_ESCAPESYSTEM_WORKING;
			packet.index = GetID();

			network.send_packet(&packet);
#endif
			Sound::GetInstance()->StopObjectSound(m_pSounds[0]);
			Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 1.0f);
			m_fCooltime = 0;
		}
		else {
			if (false == IsEqual(m_fCooltime, 0)) {
#if USE_NETWORK
				// send CheckStop();
				Network& network = *Network::GetInstance();

				cs_packet_request_escapesystem_lever_working packet;
				packet.size = sizeof(packet);
				packet.type = CS_PACKET::CS_PACKET_ESCAPESYSTEM_LEVER_WORKING;
				packet.index = GetID();
				packet.is_start = false;

				network.send_packet(&packet);
#endif
				Sound::GetInstance()->StopObjectSound(m_pSounds[0]);
				Sound::GetInstance()->PlayObjectSound(m_pSounds[1], 1.0f);
				m_fCheckCooltime = 0;
			}
			m_fCooltime = 0;
			IsInteraction = false;
		}
	}
}

void EscapeObject::SetOpen(bool open)
{
}

void EscapeObject::SetRotation(DIR d)
{
	switch (d)
	{
	case DEGREE0:
		m_dir = DEGREE0;
		break;
	case DEGREE90:
		m_dir = DEGREE90;
		Rotate(0, 90, 0);
		break;
	case DEGREE180:
		m_dir = DEGREE180;
		Rotate(0, 180, 0);
		break;
	default:
		m_dir = DEGREE270;
		Rotate(0, -90, 0);
		break;
	}
}

void EscapeObject::SetPosition(XMFLOAT3 xmf3Position)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], xmf3Position.x, xmf3Position.y, xmf3Position.z);
	}
}

void EscapeObject::SetPosition(float x, float y, float z)
{
	Sound& sound = *Sound::GetInstance();
	GameObject::SetPosition(x, y, z);
	for (int i = 0; i < m_nSound; ++i) {
		sound.SetObjectPos(m_pSounds[i], x, y, z);
	}
}

void EscapeObject::SetWorking()
{
	// 전력장치가 모두 수리 완료되었을 때 호출
	IsWorking = true; 
}
