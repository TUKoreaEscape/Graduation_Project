#include "Object.h"

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
}
