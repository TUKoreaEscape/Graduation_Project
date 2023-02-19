#pragma once
#include "Scene.h"
#include "Player.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //������ ���������� �������� ����ϱ����� �����̴�. ������ ������� �ʰ� ���Ǹ� �صξ���.


//GameScene�� Scene�� �и��س��� ����
//GameScene���� ���ӳ��� �÷��̾� ����, ������Ʈ ��ġ, ���� ���� ���� �����ϱ� ����.
class GameScene : public Scene
{
public:
	class Player* player;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL; //��Ʈ �ñ׳��ĸ� ��Ÿ���� �������̽� �������̴�.
	ID3D12PipelineState *m_pd3dPipelineState = NULL; //���������� ���¸� ��Ÿ���� �������̽� �������̴�.
public:
	GameScene();
	~GameScene() {}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);


	void BuildObjects(ID3D12Device* pd3dDevice);

	void ReleaseObjects();

	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //��Ʈ �ñ׳��ĸ� �����Ѵ�.
	void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice); //���� ���̴��� �ȼ� ���̴��� �����Ѵ�
};
