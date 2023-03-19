#pragma once
#include "stdafx.h"
#include "Component.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Shader.h"
#include "Animation.h"
#include "GameObject.h"

// 실제 게임안에 들어가는 오브젝트들(블럭, 상자 등)의 클래스들이 들어갈 곳.
// 이 오브젝트들은 GameObject들을 상속받는다.

//class CShader;
//class CStandardShader;
//class CGameObject;

class Object : virtual public GameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	Object();
	Object(int nMaterials);
	virtual ~Object();

public:
	char							m_pstrFrameName[64];

	Mesh* m_pMesh = NULL;

	int								m_nMaterials = 0;
	Material** m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	Object* m_pParent = NULL;
	Object* m_pChild = NULL;
	Object* m_pSibling = NULL;

	void SetMesh(Mesh* pMesh);
	void SetShader(Shader* pShader);
	void SetShader(int nMaterial, Shader* pShader);
	void SetMaterial(int nMaterial, Material* pMaterial);

	void SetChild(Object* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void update(float fTimeElapsed); //정의할때 부모클래스의 update도 불러줘야한다.

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, Camera* pCamera = NULL); //

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, Material* pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	Object* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	Object* FindFrame(char* pstrFrameName);

	Texture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

public:
	AnimationController* m_pAnimationController = NULL;

	Object* GetRootSkinnedGameObject();

	void SetAnimationSet(int nAnimationSet);

	void CacheSkinningBoneFrames(Object* pRootFrame);

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, Object* pParent, FILE* pInFile, Shader* pShader);
	void LoadAnimationFromFile(FILE* pInFile);

	static Object* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Object* pParent, FILE* pInFile, Shader* pShader);
	static Object* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader, bool bHasAnimation);

	static void PrintFrameInfo(Object* pGameObject, Object* pParent);
};

class HeightMapImage
{
private:
	BYTE* m_pHeightMapPixels;

	int								m_nWidth;
	int								m_nLength;
	XMFLOAT3						m_xmf3Scale;

public:
	HeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~HeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class HeightMapTerrain : public Object
{
public:
	HeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : GameObject()
	{
		//AddComponent<Texture>();
	}
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