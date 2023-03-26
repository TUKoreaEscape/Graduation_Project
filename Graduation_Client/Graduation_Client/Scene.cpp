#include "stdafx.h"
#include "Scene.h"

Scene* Scene::scene{ nullptr };

Scene::Scene()
{
	scene = this;
//	m_pd3dDevice = pd3dDevice;
	//pd3dDevice->AddRef();
	//m_pd3dCommandList = pd3dCommandList;
	//pd3dCommandList->AddRef();
}

GameObject* Scene::CreateEmpty()
{
	return new GameObject();
}

void Scene::update(float elapsedTime, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	while (!creationQueue.empty())
	{
		auto gameObject = creationQueue.front();
		gameObject->start(pd3dDevice, pd3dCommandList);
		gameObjects.push_back(gameObject);
		creationQueue.pop();
	}

	for (auto& gameObject : gameObjects) {
		gameObject->update(elapsedTime);
		//gameObject->UpdateTransform(nullptr);
	}

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
	for (auto& object : gameObjects) {
		//object->OnPrepareRender();
		object->render(pd3dCommandList);
	}
}

void Scene::AddPlayer(GameObject* player)
{
	creationQueue.push(player);
}
