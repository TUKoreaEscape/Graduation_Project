#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "GameObject.h"

GameObject::GameObject()
{
	Scene::scene->creationQueue.push(this);
}

Texture* GameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	Texture* pTexture = nullptr;
	if (renderer) if (pTexture = renderer->FindReplicatedTexture(pstrTextureName)) return pTexture;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return pTexture;
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return pTexture;
	return nullptr;
}
