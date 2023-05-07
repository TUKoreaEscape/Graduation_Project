#pragma once
#include <thread>
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "Camera.h"
#include "Object.h"
#include "Network.h"

enum E_GAME_STATE { E_GAME_OVER, E_GAME_RUNNING };
extern E_GAME_STATE gameState; //������ ���������� �������� ����ϱ����� �����̴�. ������ ������� �ʰ� ���Ǹ� �صξ���.


//GameScene�� Scene�� �и��س��� ����
//GameScene���� ���ӳ��� �÷��̾� ����, ������Ʈ ��ġ, ���� ���� ���� �����ϱ� ����.
class GameScene : public Scene
{
public:
	Player* m_pPlayer = nullptr;

	int m_nPlayers = 0;
	Player** m_ppPlayers = nullptr;

	GameObject* m_pNPC = nullptr;
	GameObject* m_pLight = nullptr;
	GameObject* m_pSkybox = nullptr;

	GameObject* m_pPianoTerrain = nullptr;
	GameObject* m_pMainTerrain = nullptr;
	GameObject* m_pBroadcastTerrain = nullptr;
	GameObject* m_pClassroomTerrain = nullptr;
	GameObject* m_pForestTerrain = nullptr;
	GameObject* m_pCubeTerrain = nullptr;

	GameObject* m_pClass = nullptr;
	GameObject* m_pPiano = nullptr;
	GameObject* m_pBroadcast = nullptr;
	GameObject* m_pPorest = nullptr;
	GameObject* m_pLobby = nullptr;

	ID3D12RootSignature*	m_pd3dGraphicsRootSignature = NULL; //��Ʈ �ñ׳��ĸ� ��Ÿ���� �������̽� �������̴�.

	GameObject* Vents[8];

	Network* m_network;
	std::thread recv_thread;
	std::thread send_thread;
	int m_nWalls;
	GameObject** m_ppWalls;


	int m_nBush;
	GameObject** m_ppBush;
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

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //��Ʈ �ñ׳��ĸ� �����Ѵ�.
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	void ReleaseUploadBuffers();

	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	void LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
	void LoadSceneBushFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
};
