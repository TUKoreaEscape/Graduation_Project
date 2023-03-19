#pragma once
#include "Scene.h"
#include "Player.h"
#include "Camera.h"
#include "GameObject.h"
#include "Object.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //게임이 진행중인지 끝났는지 사용하기위한 변수이다. 아직은 사용하지 않고 정의만 해두었다.


//GameScene과 Scene을 분리해놓은 이유
//GameScene에서 게임내의 플레이어 생성, 오브젝트 배치, 상태 등을 따로 관리하기 위해.
class GameScene : public Scene
{
public:
	Player*						m_pPlayer;
	Camera*						m_pCamera = NULL;
	HeightMapTerrain*		m_pTerrain = NULL;
	ID3D12RootSignature*	m_pd3dGraphicsRootSignature = NULL; //루트 시그너쳐를 나타내는 인터페이스 포인터이다.
	ID3D12PipelineState *	m_pd3dPipelineState = NULL; //파이프라인 상태를 나타내는 인터페이스 포인터이다.
public:
	GameScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~GameScene() {}

	virtual void render(ID3D12GraphicsCommandList* pd3dCommandList);


	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects();

	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //루트 시그너쳐를 생성한다.
	void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice); //정점 셰이더와 픽셀 셰이더를 생성한다
};
