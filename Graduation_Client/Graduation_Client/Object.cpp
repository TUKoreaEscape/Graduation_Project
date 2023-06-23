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
}

bool Vent::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	return false;
}

void Vent::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void Vent::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
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
	if (IsRot) {
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 + 1.5f;
		minz = m_xmf4x4ToParent._43 - 2.0f;
		maxz = m_xmf4x4ToParent._43 + 2.0f;
	}
	else {
		minx = m_xmf4x4ToParent._41 - 2.0f;
		maxx = m_xmf4x4ToParent._41 + 2.0f;
		minz = m_xmf4x4ToParent._43 - 1.5f;
		maxz = m_xmf4x4ToParent._43 + 1.5f;
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
	m_bOnAndOff[1] = true;
	m_bOnAndOff[4] = true;
	m_bOnAndOff[7] = true;

	GameObject* pKnob = FindFrame("Knob");

	m_pCup = FindFrame("Cup");
	m_pMainKnob = FindFrame("Main_Knob");
}

bool PowerSwitch::IsPlayerNear(const XMFLOAT3& PlayerPos)
{
	float minx, maxx, minz, maxz;
	if (IsRot) {
		minx = m_xmf4x4ToParent._41 - 1.5f;
		maxx = m_xmf4x4ToParent._41 - 0.5f;
		minz = m_xmf4x4ToParent._43 - 0.6f;
		maxz = m_xmf4x4ToParent._43 + 0.6f;
	}
	else {
		minx = m_xmf4x4ToParent._41 - 0.6f;
		maxx = m_xmf4x4ToParent._41 + 0.6f;
		minz = m_xmf4x4ToParent._43 + 0.5f;
		maxz = m_xmf4x4ToParent._43 + 1.5f;
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
	for (int i = 0; i < 15; ++i) {
		std::string str = "Knob";
		if (i != 0) {
			if (i < 10)
				str += "00" + std::to_string(i);
			else
				str += "0" + std::to_string(i);
		}
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
