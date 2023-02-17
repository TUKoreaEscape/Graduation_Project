#include "Scene.h"

Scene* Scene::scene{ nullptr };

Scene::Scene()
{
	scene = this;
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
		gameObject->start();
		gameObjects.push_back(gameObject);
		creationQueue.pop();
	}

	for (auto gameObject : gameObjects)
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
	pd3dCommandList->DrawInstanced(3, 1, 0, 0);
	/*for (auto object : Scene::scene->gameObjects)
		object->render();*/
}