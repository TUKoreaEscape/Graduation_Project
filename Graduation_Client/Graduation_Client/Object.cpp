#include "Object.h"
#include "Input.h"

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
	
	DXGI_FORMAT pdxgiRtvFormats[7] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT };
	SkyBoxShader* pSkyBoxShader = new SkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 7, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
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
}

Vent::~Vent()
{
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
	if (open) {
		if (IsOpen) return;
		Rotate(0, 90, 0);
		SetPosition(m_xmf3OpenPosition);
		UpdateTransform(NULL);
		IsOpen = true;
	}
	else {
		if (IsOpen == false) return;
		Rotate(0, -90, 0);
		SetPosition(m_xmf3ClosePosition);
		UpdateTransform(NULL);
		IsOpen = false;
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
	if (IsNear) {
		if (m_pInteractionUI) {
			if (IsRot)
				m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 0.5f, m_xmf4x4ToParent._43);
			else
				m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41, 0.5f, m_xmf4x4ToParent._43 + 0.5f);
			m_pInteractionUI->BillboardRender(pd3dCommandList, m_fPitch, m_fYaw, m_fRoll);
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
}

void Vent::Interaction(int playerType)
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
	if (IsOpen) {
		SetOpen(false);
	}
	else {
		SetOpen(true);
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

Door::Door() : InteractionObject()
{
}

Door::~Door()
{
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
		minz = m_xmf4x4ToParent._43 - 1.5f;
		maxz = m_xmf4x4ToParent._43 + 1.5f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 + 1.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 1.5f;
		maxz = m_xmf4x4ToParent._43 + 1.5f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 + 1.5f;
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
}

void Door::SetPosition(XMFLOAT3 xmf3Position)
{
	GameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);

	GameObject* leftDoor = FindFrame("Left_Door_Final");
	GameObject* rightDoor = FindFrame("Right_Door_Final");

	LeftDoorPos = leftDoor->GetPosition();
	RightDoorPos = rightDoor->GetPosition();
}

void Door::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (IsWorking) return;
	if (IsNear) {
		if (m_pInteractionUI) {
			m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41, 1.0f, m_xmf4x4ToParent._43 + 0.5f );
			m_pInteractionUI->BillboardRender(pd3dCommandList, m_fPitch, m_fYaw, m_fRoll);
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
			if (m_fCooltime >= DOOR_CLOSE_COOLTIME_PLYAER) {
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
			IsInteraction = false;
			m_fCooltime = 0;
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
			if (m_fCooltime >= DOOR_OPEN_COOLTIME_PLYAER) {
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
			IsInteraction = false;
			m_fCooltime = 0;
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
		}
	}
	else {
		if (Open == true) {
			IsOpen = true;
			IsWorking = true;
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

DoorUI::DoorUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	Mesh* pUIMesh = new TexturedRectMesh(pd3dDevice, pd3dCommandList, -0.5, -0.5, 1, 1);
	SetMesh(pUIMesh);

	Texture* pUITexture = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pUITexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pstrFileName, RESOURCE_TEXTURE2D, 0);

	GameScene::CreateShaderResourceViews(pd3dDevice, pUITexture, 0, 17);

	Material* pUIMaterial = new Material(1);
	pUIMaterial->SetTexture(pUITexture);
	pUIMaterial->SetDoorUIShader();

	renderer->SetMaterial(0, pUIMaterial);
}

DoorUI::~DoorUI()
{
}

void DoorUI::BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT3 xmf3CameraPosition = Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition();

	SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));

	render(pd3dCommandList);
}

void DoorUI::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

InteractionObject::InteractionObject() : GameObject()
{
}

InteractionObject::~InteractionObject()
{
}

void InteractionObject::SetUI(InteractionUI* ui)
{
	if (m_pInteractionUI) m_pInteractionUI->Release();
	m_pInteractionUI = ui;
	if (m_pInteractionUI) m_pInteractionUI->AddRef();
}

InteractionUI::InteractionUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	Mesh* pUIMesh = new TexturedRectMesh(pd3dDevice, pd3dCommandList, -0.5, -0.5, 1, 1);
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

void InteractionUI::BillboardRender(ID3D12GraphicsCommandList* pd3dCommandList, float x, float y, float z)
{
	XMFLOAT3 xmf3CameraPosition = Input::GetInstance()->m_pPlayer->m_pCamera->GetPosition();

	SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));

	Rotate(x, y, z);

	UpdateTransform(nullptr);

	render(pd3dCommandList);

	Rotate(-x, -y, -z);
}

void InteractionUI::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

PowerSwitch::PowerSwitch() : InteractionObject()
{
	IsOpen = false;
}

PowerSwitch::~PowerSwitch()
{
}

void PowerSwitch::Init()
{
	GameObject* pKnob = FindFrame("Knob");

	m_pCup = FindFrame("Cup");
	m_pMainKnob = FindFrame("Main_Knob");
}

bool PowerSwitch::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	switch (m_dir) {
	case DEGREE0:
		minx = m_xmf4x4ToParent._41 - 0.6f;
		maxx = m_xmf4x4ToParent._41 + 0.6f;
		minz = m_xmf4x4ToParent._43 + 0.5f;
		maxz = m_xmf4x4ToParent._43 + 1.5f;
		break;
	case DEGREE90:
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 - 0.5f;
		minz = m_xmf4x4ToParent._43 - 0.6f;
		maxz = m_xmf4x4ToParent._43 + 0.6f;
		break;
	case DEGREE180:
		minx = m_xmf4x4ToParent._41 - 0.6f;
		maxx = m_xmf4x4ToParent._41 + 0.6f;
		minz = m_xmf4x4ToParent._43 - 1.5f;
		maxz = m_xmf4x4ToParent._43 - 0.5f;
		break;
	default:
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 - 0.5f;
		minz = m_xmf4x4ToParent._43 - 0.6f;
		maxz = m_xmf4x4ToParent._43 + 0.6f;
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
}

void PowerSwitch::SetActivate(bool value)
{
	m_bClear = value;
}

void PowerSwitch::update(float fElapsedTime)
{
	if (m_bClear) return;
	if (m_bIsOperating == false) return;
	if (IsNear == false) {
		m_bIsOperating = false;
		return;
	}
	m_fCooltime += fElapsedTime;
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
	}
	if (keyBuffer['v'] & 0xF0 || keyBuffer['V'] & 0xF0) {
		if (false == CheckAnswer()) {
			Reset();
			m_bIsOperating = false;
			return;
		}
		m_bClear = true;
		m_bIsOperating = false;
#if USE_NETWORK
		cs_packet_request_electronic_system_activate packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_REQUEST_ELETRONIC_SYSTEM_ATIVATE;
		packet.system_index = m_switch_index;
		Network& network = *Network::GetInstance();
		network.send_packet(&packet);
#endif
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
				m_fOnKnobPos -= 0.2;
				pKnob->m_xmf4x4ToParent._41 = -0.21;
			}
			else {		
				pKnob->m_xmf4x4ToParent._41 = 0.07291567;
			}
		}
	}
	UpdateTransform(nullptr);
	if (m_bClear) {
		FindFrame("Lamp_1")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		FindFrame("Lamp_2")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	}
	else {
		FindFrame("Lamp_1")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 0.5f, 0.04827571f, 1.0f);
		FindFrame("Lamp_2")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	GameObject::render(pd3dCommandList);
}

void PowerSwitch::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_bClear) return;
	if (IsNear) {
		if (m_pInteractionUI) {
			if (IsRot)
				m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41 + 0.5f, 1.5f, m_xmf4x4ToParent._43);
			else
				m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41, 1.5f, m_xmf4x4ToParent._43 + 0.5f);
			m_pInteractionUI->BillboardRender(pd3dCommandList, m_fPitch, m_fYaw, m_fRoll);
		}
	}
}

void PowerSwitch::Interaction(int playerType)
{
	if (m_bClear) return;
	if (m_bIsOperating) return;
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
			m_bIsOperating = true;
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
	m_dir = DEGREE0;
	for (int i = 0; i < 6; ++i) {
		m_pItems[i] = nullptr;
	}
}

ItemBox::~ItemBox()
{
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
	if (IsNear) {
		if (m_pInteractionUI) {
			m_pInteractionUI->SetPosition(m_xmf4x4ToParent._41, 0.5f, m_xmf4x4ToParent._43 + 0.5f);
			m_pInteractionUI->BillboardRender(pd3dCommandList, m_fPitch, m_fYaw, m_fRoll);
		}
	}
}

void ItemBox::Interaction(int playerType)
{
	if (IsOpen) {
		switch (playerType) {
		case TYPE_TAGGER:
			SetOpen(false);
		case TYPE_PLAYER_YET:
		case TYPE_DEAD_PLAYER:
		case TYPE_PLAYER:
			break;
		}
	}
	else {
		switch (playerType) {
		case TYPE_TAGGER:
		case TYPE_PLAYER_YET:
			break;
		case TYPE_DEAD_PLAYER:
			SetOpen(true); // tempa
			break;
		case TYPE_PLAYER:
			SetOpen(true);
			break;
		}
	}
#if USE_NETWORK
	Network& network = *Network::GetInstance();
	network.Send_Fix_Object_Box_Update(m_item_box_index, IsOpen);
#endif
}

void ItemBox::SetOpen(bool open)
{
	if (false == open) {
		if (false == IsOpen) return;
		IsOpen = false;
	}
	else {
		if (true == IsOpen) return;
		IsOpen = true;
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