#pragma once
#include "Component.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Animation.h"

class GameObject
{
	std::list<Component*> components;

private:
	int m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	GameObject();
	GameObject(int nMaterials);
	virtual void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	{
		for (auto& component : components)
			component->start(pd3dDevice, pd3dCommandList);
		//UpdateTransform(nullptr);
	}

	virtual void update(float elapsedTime)
	{
		if (m_pSibling) m_pSibling->update(elapsedTime);
		if (m_pChild) m_pChild->update(elapsedTime);
		for (auto& component : components)
			component->update(elapsedTime);
	}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void OnPrepareRender() {}

	template<typename T>
	T* AddComponent();

	template<typename T>
	T* GetComponent()
	{
		for (auto& component : components)
		{
			auto c = dynamic_cast<T*>(component);
			if (c) return c;
		}
		return nullptr;
	}

	Texture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	void SetMesh(Mesh* pMesh);
	void SetChild(GameObject* pChild, bool bReferenceUpdate = false);

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = nullptr);

	virtual void ReleaseUploadBuffers();

public:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

public:
	char m_pstrFrameName[64];

	XMFLOAT4X4 m_xmf4x4World;
	XMFLOAT4X4 m_xmf4x4ToParent;

	GameObject* m_pParent = nullptr;
	GameObject* m_pSibling = nullptr;
	GameObject* m_pChild = nullptr;

	Mesh* m_pMesh = nullptr;

	StandardRenderer* renderer = nullptr;

public:
	bool isNotDraw = false;

	void SetNotDraw();

public:
	AnimationController* m_pSkinnedAnimationController = nullptr;

	void CacheSkinningBoneFrames(GameObject* pRootFrame);

	//static GameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObject* pParent, FILE* pInFile, Shader* pShader);
	
	static void PrintFrameInfo(GameObject* pGameObject, GameObject* pParent);
	GameObject* FindFrame(char* pstrFrameName);

public:
	SkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	
	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	static void LoadAnimationFromFile(FILE* pInFile, LoadedModelInfo* pLoadedModel);
	
	static GameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObject* pParent, FILE* pInFile, Shader* pShader, int* pnSkinnedMeshes);

	static LoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader);

	void Animate(float fTimeElapsed);

	void FindCustomPart(const char* pstrFrameName);
};

template<typename T>
T* GameObject::AddComponent()
{
	auto component = new T;
	component->gameObject = this;
	components.push_back(component);
	return component;
}