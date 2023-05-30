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

Vent::Vent() : GameObject()
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

Door::Door() : GameObject()
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
	IsRot = true;

}

bool Door::CheckDoor(const XMFLOAT3& PlayerPos)
{
	IsOpen = false;
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
	if (PlayerPos.x > maxx) return false;
	if (PlayerPos.x < minx) return false;
	if (PlayerPos.z < minz) return false;
	if (PlayerPos.z > maxz) return false;
	IsOpen = true;
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
			//IsOpen = false;
		}
	}
	else {
		OpenTime -= fElapsedTime;
		if (OpenTime <= 0.0f) {
			OpenTime = 0.0f;
			//IsOpen = true;
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
	if (1) {
		if (m_pDoorUI)
			m_pDoorUI->render(pd3dCommandList);
	}
}

UIObject::UIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, wchar_t* pstrFileName)
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	renderer->m_nMaterials = 1;
	renderer->m_ppMaterials = new Material * [renderer->m_nMaterials];
	renderer->m_ppMaterials[0] = new Material(0);

	UIMesh* pUIMesh = new UIMesh(pd3dDevice, pd3dCommandList);
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
