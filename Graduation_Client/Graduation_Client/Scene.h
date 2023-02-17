#pragma once
#include "stdafx.h"
#include "GameObject.h"

class Scene
{
	std::queue<GameObject*> creationQueue; //�����ϰ���� ������Ʈ�� �߰��ϴ� ť�̴�.
	std::deque<GameObject*> deletionQueue;// �����ϰ���� ������Ʈ�� �߰��ϴ� ���̴�.

	std::list<GameObject*> gameObjects; //���� ������Ʈ���� �� ����Ʈ�� ���� �ȴ�.
	//�� ���� �и��س��� ����
	//� ������Ʈ�� ������Ʈ ���߿� �߰��� �Ǵ°��, �����Ǵ� ��쿡 ���α׷��� � ������ ��ĥ�� �𸣱⶧����
	//������Ʈ�� �ϱ� �� creationQueue�� ���� �߰��Ǵ� ������Ʈ���� ����ִ��� Ȯ���� �� ������Ʈ�� ��� ��ģ �Ŀ� 
	//deletionQueue���� �����ϰ� ���� ������Ʈ���� gameObjects���� ã�Ƽ� �����ϴ� ���·� �����ߴ�.

public:
	static Scene* scene;

protected:
	GameObject* CreateEmpty();

public:
	Scene();
	virtual ~Scene()
	{
		for (auto object : gameObjects)
			delete object;
		gameObjects.clear();
	}
	virtual void update(float elapsedTime);
	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);

	void PushDelete(GameObject* gameObject)
	{
		if (std::find(deletionQueue.begin(), deletionQueue.end(), gameObject) == deletionQueue.end()) deletionQueue.push_back(gameObject);
	}

	friend GameObject;
};
