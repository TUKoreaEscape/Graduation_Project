#pragma once
#include "Component.h"
#include "Texture.h"
#include "Material.h"

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World;
};

class StandardRenderer : public Component
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void update(float elapsedTime) {}

	void render(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObject* pParent, FILE* pInFile, Shader* pShader);
	Texture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	void SetMaterial(int nMaterial, Material* pMaterial);
	void SetMaterial(Material* pMaterial);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList) {}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {}
	virtual void ReleaseShaderVariables() {}

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	void ReleaseUploadBuffers();
	
	UINT GetMeshType();

	int m_nMaterials = 0;
	Material** m_ppMaterials = nullptr;
};

class SkyboxRenderer : public StandardRenderer
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void update(float elapsedTime) {}
	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
};