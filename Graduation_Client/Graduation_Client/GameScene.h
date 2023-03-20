#pragma once
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "Camera.h"
#include "Object.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //������ ���������� �������� ����ϱ����� �����̴�. ������ ������� �ʰ� ���Ǹ� �صξ���.


//GameScene�� Scene�� �и��س��� ����
//GameScene���� ���ӳ��� �÷��̾� ����, ������Ʈ ��ġ, ���� ���� ���� �����ϱ� ����.
class GameScene : public Scene
{
public:
	GameObject* m_pPlayer = nullptr;
	GameObject*						m_pCamera = NULL;
	GameObject* m_pLight = nullptr;
	//HeightMapTerrain*		m_pTerrain = NULL;
	ID3D12RootSignature*	m_pd3dGraphicsRootSignature = NULL; //��Ʈ �ñ׳��ĸ� ��Ÿ���� �������̽� �������̴�.

public:
	GameScene();
	~GameScene() {}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);


	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //��Ʈ �ñ׳��ĸ� �����Ѵ�.
};
