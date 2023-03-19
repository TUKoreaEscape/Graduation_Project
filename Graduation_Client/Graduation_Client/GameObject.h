#pragma once
#include "Component.h"
#include "Mesh.h"
#include "Renderer.h"

class GameObject
{
	std::list<Component*> components;

public:
	GameObject();
	virtual void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	{
		for (auto& component : components)
			component->start(pd3dDevice, pd3dCommandList);
	}

	virtual void update(float elapsedTime)
	{
		for (auto& component : components)
			component->update(elapsedTime);
	}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList) { }

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

public:
	XMFLOAT4X4 m_xmf4x4World;
	XMFLOAT4X4 m_xmf4x4ToParent;

	GameObject* m_pParent;
	GameObject* m_pSibling;
	GameObject* m_pChild;

	Mesh* m_pMesh;

	StandardRenderer* renderer; 
};

template<typename T>
T* GameObject::AddComponent()
{
	auto component = new T;
	component->gameObject = this;
	components.push_back(component);
	return component;
}