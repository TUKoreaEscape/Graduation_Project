#pragma once
#include "Texture.h"
#include "Shader.h"

class GameObject;

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

class Material
{
public:
	Material(int nTextures);
	virtual ~Material();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	Shader* m_pShader = nullptr;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetShader(Shader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(Texture* pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseUploadBuffers();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, Texture** ppTexture, GameObject* pParent, FILE* pInFile, Shader* pShader);

public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	Texture** m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

public:
	static Shader* m_pStandardShader;
	static Shader* m_pSkinnedAnimationShader;
	static Shader* m_pBushShader;
	static Shader* m_pTerrainShader;
	static Shader* m_pUIShader;
	static Shader* m_pDoorUIShader;
	static void Material::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void SetStandardShader() { Material::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { Material::SetShader(m_pSkinnedAnimationShader); }
	void SetBushShader() { Material::SetShader(m_pBushShader); }
	void SetTerrainShader() { Material::SetShader(m_pTerrainShader); }
	void SetUIShader() { Material::SetShader(m_pUIShader); }
	void SetDoorUIShader() { Material::SetShader(m_pDoorUIShader); }
};