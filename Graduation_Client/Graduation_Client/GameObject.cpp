#pragma once
#include "GameObject.h"
#include "Scene.h"

GameObject::GameObject()
{
	Scene::scene->creationQueue.push(this);
}