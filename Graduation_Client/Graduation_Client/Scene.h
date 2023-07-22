#pragma once
#include "GameObject.h"

class GameObject;

class Scene
{
	std::queue<GameObject*> creationQueue; //�����ϰ���� ������Ʈ�� �߰��ϴ� ť�̴�.
	std::deque<GameObject*> deletionQueue;// �����ϰ���� ������Ʈ�� �߰��ϴ� ���̴�.

	std::list<GameObject*> gameObjects; //���� ������Ʈ���� �� ����Ʈ�� ���� �ȴ�.
	//�� ���� �и��س��� ����
	//� ������Ʈ�� ������Ʈ ���߿� �߰��� �Ǵ°��, �����Ǵ� ��쿡 ���α׷��� � ������ ��ĥ�� �𸣱⶧����
	//������Ʈ�� �ϱ� �� creationQueue�� ���� �߰��Ǵ� ������Ʈ���� ����ִ��� Ȯ���� �� ������Ʈ�� ��� ��ģ �Ŀ� 
	//deletionQueue���� �����ϰ� ���� ������Ʈ���� gameObjects���� ã�Ƽ� �����ϴ� ���·� �����ߴ�.
	//ID3D12Device* m_pd3dDevice = NULL;
	//ID3D12GraphicsCommandList* m_pd3dCommandList=NULL;

public:
	static Scene* scene;

protected:
	GameObject* CreateEmpty();

public:
	Scene();
	virtual ~Scene()
	{
		for (auto& object : gameObjects) {
			object->Release();
		}
		gameObjects.clear();
	}
	virtual void update(float elapsedTime, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Depthrender(ID3D12GraphicsCommandList* pd3dCommandList);

	void PushDelete(GameObject* gameObject)
	{
		if (std::find(deletionQueue.begin(), deletionQueue.end(), gameObject) == deletionQueue.end()) deletionQueue.push_back(gameObject);
	}

	void AddPlayer(GameObject* player);

	friend GameObject;

	float m_fElapsedTime = 0.0f;
};
