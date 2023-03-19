#include "stdafx.h"
#include "Scene.h"

Scene* Scene::scene{ nullptr };

Scene::Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	scene = this;
	m_pd3dDevice = pd3dDevice;
	m_pd3dCommandList = pd3dCommandList;
}

GameObject* Scene::CreateEmpty()
{
	return new GameObject();
}

void Scene::update(float elapsedTime)
{
	while (!creationQueue.empty())
	{
		auto gameObject = creationQueue.front();
		gameObject->start(m_pd3dDevice, m_pd3dCommandList);
		gameObjects.push_back(gameObject);
		creationQueue.pop();
	}

	for (auto& gameObject : gameObjects)
		gameObject->update(elapsedTime);

	auto t = deletionQueue;
	while (!deletionQueue.empty())
	{
		auto gameObject = deletionQueue.front();
		gameObjects.erase(std::find(gameObjects.begin(), gameObjects.end(), gameObject));
		deletionQueue.pop_front();

		delete gameObject;
	}
}

void Scene::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (auto& object : gameObjects)
		object->render(pd3dCommandList);
}