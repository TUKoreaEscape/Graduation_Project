#pragma once
#include "GameObject.h"

class GameObject;

class Scene
{
	std::queue<GameObject*> creationQueue; //생성하고싶은 오브젝트를 추가하는 큐이다.
	std::deque<GameObject*> deletionQueue;// 삭제하고싶은 오브젝트를 추가하는 덱이다.

	std::list<GameObject*> gameObjects; //실제 오브젝트들은 이 리스트에 담기게 된다.
	//이 셋을 분리해놓은 이유
	//어떤 오브젝트가 업데이트 도중에 추가가 되는경우, 삭제되는 경우에 프로그램에 어떤 영향을 미칠지 모르기때문에
	//업데이트를 하기 전 creationQueue에 새로 추가되는 오브젝트들이 담겨있는지 확인한 후 업데이트를 모두 마친 후에 
	//deletionQueue에서 삭제하고 싶은 오브젝트들을 gameObjects에서 찾아서 삭제하는 형태로 제작했다.
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
