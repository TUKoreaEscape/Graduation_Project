#pragma once
#include "Component.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Animation.h"

class GameObject
{
	std::list<Component*> components;

public:
	GameObject();
	virtual void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	{
		for (auto& component : components)
			component->start(pd3dDevice, pd3dCommandList);
		UpdateTransform(nullptr);
	}

	virtual void update(float elapsedTime)
	{
		if (m_pAnimationController) m_pAnimationController->AdvanceTime(elapsedTime, NULL);
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

	static GameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObject* pParent, FILE* pInFile, Shader* pShader);
	static GameObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader);

	void SetMesh(Mesh* pMesh);
	void SetChild(GameObject* pChild);

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = nullptr);

	virtual void ReleaseUploadBuffers();

public:
	char m_pstrFrameName[64];

	XMFLOAT4X4 m_xmf4x4World;
	XMFLOAT4X4 m_xmf4x4ToParent;

	GameObject* m_pParent;
	GameObject* m_pSibling;
	GameObject* m_pChild;

	Mesh* m_pMesh;

	StandardRenderer* renderer; 

public:
	AnimationController* m_pAnimationController = nullptr;

	GameObject* GetRootSkinnedGameObject();
	
	void SetAnimationSet(int nAnimationSet);

	void CacheSkinningBoneFrames(GameObject* pRootFrame);

	void LoadAnimationFromFile(FILE* pInFile);

	//static GameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObject* pParent, FILE* pInFile, Shader* pShader);
	static GameObject* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, Shader* pShader, bool bHasAnimation);

	static void PrintFrameInfo(GameObject* pGameObject, GameObject* pParent);
	GameObject* FindFrame(char* pstrFrameName);
};

template<typename T>
T* GameObject::AddComponent()
{
	auto component = new T;
	component->gameObject = this;
	components.push_back(component);
	return component;
}