#pragma once
#include <thread>
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "Camera.h"
#include "Object.h"
#include "Network.h"
#include "Game_state.h"

enum class ROOM_INFO {
	CLASS_ROOM0 = 0,
	CLASS_ROOM1,
	PIANO_ROOM0,
	PIANO_ROOM1,
	BROADCASTING_ROOM0,
	BROADCASTING_ROOM1,
	BROADCASTING_ROOM2,
	BROADCASTING_ROOM3,
	BROADCASTING_ROOM4,
	LOBBY_ROOM0,
	LOBBY_ROOM1,
	LOBBY_ROOM2,
	FOREST0,
	FOREST1,
	CUBE_ROOM,
	TOTAL_ROOM
};

enum class PVSROOM
{
	CLASS_ROOM = 0,
	PIANO_ROOM,
	BROADCASTING_ROOM,
	LOBBY_ROOM,
	FOREST,
	CUBE_ROOM
};

class InteractionUI;
class InteractionObject;
class ItemBox;
class ParticleObject;
//GameScene과 Scene을 분리해놓은 이유
//GameScene에서 게임내의 플레이어 생성, 오브젝트 배치, 상태 등을 따로 관리하기 위해.
class GameScene : public Scene
{
public:
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;

	Player* m_pPlayer = nullptr;

	int m_nPlayers = 0;
	Player** m_ppPlayers = nullptr;

	GameObject* m_pLight = nullptr;
	LIGHT* m_pLights = nullptr;
	GameObject* m_pSkybox = nullptr;

	GameObject* m_pPianoTerrain = nullptr;
	GameObject* m_pMainTerrain = nullptr;
	GameObject* m_pBroadcastTerrain = nullptr;
	GameObject* m_pClassroomTerrain = nullptr;
	GameObject* m_pForestTerrain = nullptr;
	GameObject* m_pCubeTerrain = nullptr;

	int m_nLogin;
	int m_nRoomSelect;
	int m_nWaitingRoom;
	int m_nCustomizing;
	int m_Ending;
	int m_nPlay;
	int m_nPlayPlayer;
	int m_nPlayTagger;
	int m_nLoading;

	GameObject** m_UILogin = nullptr;
	GameObject** m_UIRoomSelect = nullptr;
	GameObject** m_UIWaitingRoom = nullptr;
	GameObject** m_UICustomizing = nullptr;
	GameObject** m_UIEnding = nullptr;
	GameObject** m_UIPlay = nullptr;
	GameObject** m_UIPlayer = nullptr;
	GameObject** m_UITagger = nullptr;
	GameObject** m_UILoading = nullptr;

	GameObject* m_pCeilling = nullptr;

	int m_nWalls;
	GameObject** m_ppWalls;
	
	GameObject* Vents[NUM_VENT];

	int m_nBush;
	GameObject** m_ppBush;
	GameObject* m_pOak;	

	Door* m_pDoors[NUM_DOOR];

	InteractionObject* m_pPowers[NUM_POWER];
	ItemBox* m_pBoxes[NUM_ITEMBOX];
	Network* m_network;
	std::thread recv_thread;
	std::thread send_thread;

	InteractionObject* Taggers = nullptr;
	InteractionObject* EscapeLevers[NUM_ESCAPE_LEVER];

	std::set<ROOM_INFO> m_sPVS[6];
	PVSROOM m_pvsCamera;
	int m_nPVSRooms;
	GameObject** m_pPVSObjects;

	GameObject* Items[NUM_ITEMS];

	InteractionUI** m_ppObjectsUIs = nullptr;
	int m_nObjectsUIs{};

	int m_nSpectator = 0;

	int m_nAnswerUI{};
	GameObject** m_ppAnswerUIs = nullptr;

	int m_nInfoUI{};
	GameObject** m_ppInfoUIs = nullptr;

	float m_fInfoTime{};
protected:
	static ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
public:
	GameScene();
	~GameScene() {}

	virtual void prerender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void defrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void forrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UIrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void WaitingRoomrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Endingrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Powerrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Loadingrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void SpectatorPrerender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Spectatorrender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Depthrender(ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //루트 시그너쳐를 생성한다.
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	void ReleaseUploadBuffers();

	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	void LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
	void LoadSceneBushFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);

	void MakeVents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void MakeDoors(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void MakePowers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void MakeBoxes(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void MakeTaggers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void MakeEscapeLevers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	
	virtual void update(float elapsedTime, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void CheckCameraPos(const XMFLOAT3 camera);

	void BuildObjectsThread(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	friend class Framework;
	friend class LaplacianEdgeShader;

public:
	ParticleObject** m_ppParticleObjects = NULL;
	int							m_nParticleObjects = 0;

	void RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList);
	void OnPostRenderParticle();
};
