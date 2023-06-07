#include "stdafx.h"
#include "Light.h"
#include "GameScene.h"
#include "Movement.h"
#include "Network.h"

ID3D12DescriptorHeap* GameScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	GameScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	GameScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	GameScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	GameScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	GameScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	GameScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	GameScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	GameScene::m_d3dSrvGPUDescriptorNextHandle;
GameScene::GameScene() : Scene()
{
}

void GameScene::forrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	//if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	//m_pPlayer->m_pCamera->update(pd3dCommandList);
	//m_pLight->GetComponent<Light>()->update(pd3dCommandList);

	for (int i = 0; i < m_nBush; ++i)
	{
		if (m_ppBush[i]) m_ppBush[i]->render(pd3dCommandList);
	}
}

void GameScene::prerender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->m_pCamera->update(pd3dCommandList);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);
}

void GameScene::defrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pSkybox->render(pd3dCommandList);

	m_pCeilling->render(pd3dCommandList);

	m_pMainTerrain->render(pd3dCommandList);
	m_pPianoTerrain->render(pd3dCommandList);
	m_pBroadcastTerrain->render(pd3dCommandList);
	m_pCubeTerrain->render(pd3dCommandList);
	m_pForestTerrain->render(pd3dCommandList);
	m_pClassroomTerrain->render(pd3dCommandList);

	for (int i = 0; i < m_nWalls; ++i)
	{
		if (m_ppWalls[i]) m_ppWalls[i]->render(pd3dCommandList);
	}

	Scene::render(pd3dCommandList);
	
	m_pClass->render(pd3dCommandList);
	m_pPiano->render(pd3dCommandList);
	m_pBroadcast->render(pd3dCommandList);
	m_pPorest->render(pd3dCommandList);
	m_pLobby->render(pd3dCommandList);

	for (int i = 0; i < 8; ++i)
	{
		if (Vents[i]) {
			Vents[i]->UpdateTransform(nullptr);
			Vents[i]->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < 6; ++i)
	{
		if (m_pDoors[i]) {
			m_pDoors[i]->UpdateTransform(nullptr);
			m_pDoors[i]->render(pd3dCommandList);
		}
	}

	m_pElectric->render(pd3dCommandList);

	m_pOak->render(pd3dCommandList);
	for (int i = 0; i < m_nBush; ++i)
	{
		if (m_ppBush[i]) m_ppBush[i]->render(pd3dCommandList);
	}
	//m_pHouse->render(pd3dCommandList);
}

void GameScene::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->m_pCamera->update(pd3dCommandList);
	
	switch (GameState::GetInstance()->GetGameState()) {
	case LOGIN:
		for (int i = 0; i < m_nLogin; ++i) m_UILogin[i]->render(pd3dCommandList);
		break;
	case ROOM_SELECT:
		for (int i = 0; i < m_nRoomSelect; ++i) m_UIRoomSelect[i]->render(pd3dCommandList);
		break;
	case READY_TO_GAME:
		for (int i = 0; i < 6; ++i) {
			reinterpret_cast<Door*>(m_pDoors[i])->UIrender(pd3dCommandList);
		}
		break;
	}
}

void GameScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 150);

	Material::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	LoadedModelInfo* pPlayerModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/C33.bin", nullptr); 
	LoadedModelInfo* pClassModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InClassObject.bin", nullptr);
	LoadedModelInfo* pPianoModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InPianoRoom.bin", nullptr);
	LoadedModelInfo* pBroadcastModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InBroadcast.bin", nullptr);
	LoadedModelInfo* pHouseModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InPorest.bin", nullptr);
	LoadedModelInfo* pLobbyModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InDDD.bin", nullptr);
	LoadedModelInfo* pCeilModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Ceilling.bin", nullptr);

	//

	LoadedModelInfo* pElecModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Shield2.bin", nullptr);

	m_pElectric = new GameObject();
	m_pElectric->SetChild(pElecModel->m_pModelRootObject, true);
	m_pElectric->UpdateTransform(nullptr);

	m_pElectric->FindFrame("Lamp_2")->renderer->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	if (pElecModel) delete pElecModel;
	//

	m_nPlayers = 5;
	m_ppPlayers = new Player * [m_nPlayers];
	for (int i = 0; i < m_nPlayers; ++i) {
		m_ppPlayers[i] = new Player();
		m_ppPlayers[i]->SetChild(pPlayerModel->m_pModelRootObject, true);
		m_ppPlayers[i]->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
		m_ppPlayers[i]->SetPosition(XMFLOAT3(0, 5.f, i * 3.f));
		for (int j = 0; j < 6; ++j)
			GameObject::SetParts(i + 1, j, 0);
		m_ppPlayers[i]->PlayerNum = i + 1;
		m_ppPlayers[i]->SetPlayerType(TYPE_DEAD_PLAYER);
	}
	m_pPlayer = new Player();
	m_pPlayer->SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pPlayer->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));
	for (int j = 0; j < 6; ++j)
		GameObject::SetParts(0, j, 0);
	GameObject::SetParts(0, 0, 4);
	m_pPlayer->PlayerNum = 0;

	m_pLight = new GameObject();
	m_pLight->AddComponent<Light>();
	m_pLight->start(pd3dDevice, pd3dCommandList);
	m_pSkybox = new SkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	
	XMFLOAT3 xmf3Scale(1.0f, 1.0f, 1.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 0.0f);
	m_pMainTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 121, 81, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");
	m_pPianoTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -30, 60, 61, 41, xmf3Scale, xmf4Color, L"Terrain/Floor2.dds");
	m_pBroadcastTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 50, 60, 101, 41, xmf3Scale, xmf4Color, L"Terrain/Floor2.dds");
	m_pCubeTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 80, 0, 41, 81, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");
	m_pForestTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 60, -60, 81, 41, xmf3Scale, xmf4Color, L"Terrain/Floor2.dds");
	m_pClassroomTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -20, -60, 81, 41, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");

	//UI생성 영역 dds파일 다음 x,y,width,height가 순서대로 들어간다. 아무것도 넣지않으면 화면중앙에 1x1사이즈로 나온다.
	m_nLogin = 1;
	m_UILogin = new GameObject * [m_nLogin];
	m_UILogin[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Login.dds", 0.0f, 0.0f, 2.0f, 2.0f);

	m_nRoomSelect = 9;
	m_UIRoomSelect = new GameObject * [m_nRoomSelect];
	m_UIRoomSelect[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/RoomSelect.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UIRoomSelect[1] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/LArrow.dds", -0.2f, -0.9f, 0.2f, 0.2f);
	m_UIRoomSelect[2] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/RArrow.dds", 0.2f, -0.9f, 0.2f, 0.2f);

	m_UIRoomSelect[3] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", 0.4f, 0.7f, 0.5f, 0.4f);
	m_UIRoomSelect[4] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", 0.4f, 0.1f, 0.5f, 0.4f);
	m_UIRoomSelect[5] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", 0.4f, -0.5f, 0.5f, 0.4f);
	m_UIRoomSelect[6] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", -0.4f, 0.7f, 0.5f, 0.4f);
	m_UIRoomSelect[7] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", -0.4f, 0.1f, 0.5f, 0.4f);
	m_UIRoomSelect[8] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/blank.dds", -0.4f, -0.5f, 0.5f, 0.4f);

	LPVOID m_pTerrain[ROOM_COUNT]{ m_pMainTerrain ,m_pPianoTerrain,m_pBroadcastTerrain, m_pCubeTerrain ,m_pForestTerrain,m_pClassroomTerrain };
	
	m_pCeilling = new GameObject();
	m_pCeilling->SetChild(pCeilModel->m_pModelRootObject, true);
	m_pCeilling->UpdateTransform(nullptr);
	LoadSceneObjectsFromFile(pd3dDevice, pd3dCommandList, (char*)"Walls/Scene0523.bin");

	m_pClass = new GameObject();
	m_pClass->SetChild(pClassModel->m_pModelRootObject, true);
	m_pClass->UpdateTransform(nullptr);
	m_pPiano = new GameObject();
	m_pPiano->SetChild(pPianoModel->m_pModelRootObject, true);
	m_pPiano->UpdateTransform(nullptr);
	m_pBroadcast = new GameObject();
	m_pBroadcast->SetChild(pBroadcastModel->m_pModelRootObject, true);
	m_pBroadcast->UpdateTransform(nullptr);
	m_pLobby = new GameObject();
	m_pLobby->SetChild(pLobbyModel->m_pModelRootObject, true);
	m_pLobby->UpdateTransform(nullptr);
	m_pPorest = new GameObject();
	m_pPorest->SetChild(pHouseModel->m_pModelRootObject, true);
	m_pPorest->UpdateTransform(nullptr);

	MakeVents(pd3dDevice, pd3dCommandList);
	MakeDoors(pd3dDevice, pd3dCommandList);

	LoadSceneBushFromFile(pd3dDevice, pd3dCommandList, (char*)"Model/Bush.bin");

	m_pPlayer->SetPlayerUpdatedContext(m_pTerrain);
	m_pPlayer->SetPlayerType(TYPE_PLAYER);
	m_pPlayer->AddComponent<CommonMovement>(); 
	
	for (int i = 0; i < m_nPlayers; ++i) {
		AddPlayer(m_ppPlayers[i]);
	}
	AddPlayer(m_pPlayer);

	if (pPlayerModel) delete pPlayerModel;
	if (pClassModel) delete pClassModel;
	if (pPianoModel) delete pPianoModel;
	if (pBroadcastModel) delete pBroadcastModel;
	if (pHouseModel) delete pHouseModel;
	if (pLobbyModel) delete pLobbyModel;
	if (pCeilModel) delete pCeilModel;

#if USE_NETWORK
	char id[20]{};
	char pw[20]{};
	int select;

	m_network = Network::GetInstance();
	m_network->init_network();
	m_network->m_pPlayer = m_pPlayer;
	m_network->m_ppOther = m_ppPlayers;
	
	for(int i = 0; i < 6; ++i)
		m_network->m_pDoors[i] = m_pDoors[i];

	recv_thread = std::thread{ &Network::listen_thread, m_network };
#endif
}

void GameScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppWalls) {
		for (int i = 0; i < m_nWalls; ++i) {
			if (m_ppWalls[i]) m_ppWalls[i]->Release();
		}
		delete[] m_ppWalls;
	}
	if (m_pLight) m_pLight->Release();
	if (m_pSkybox) m_pSkybox->Release();

	if (m_pPianoTerrain) m_pPianoTerrain->Release();
	if (m_pMainTerrain) m_pMainTerrain->Release();
	if (m_pBroadcastTerrain) m_pBroadcastTerrain->Release();
	if (m_pClassroomTerrain) m_pClassroomTerrain->Release();
	if (m_pForestTerrain) m_pForestTerrain->Release();
	if (m_pCubeTerrain) m_pCubeTerrain->Release();
	if (m_UILogin) {
		for (int i = 0; i < m_nLogin; ++i) {
			if (m_UILogin[i]) m_UILogin[i]->Release();
		}
		delete[] m_UILogin;
	}
	if (m_UIRoomSelect) {
		for (int i = 0; i < m_nRoomSelect; ++i) {
			if (m_UIRoomSelect[i]) m_UIRoomSelect[i]->Release();
		}
		delete[] m_UIRoomSelect;
	}

	if (m_pClass) m_pClass->Release();
	if (m_pPiano) m_pPiano->Release();
	if (m_pBroadcast) m_pBroadcast->Release();
	if (m_pPorest) m_pPorest->Release();
	if (m_pLobby) m_pLobby->Release();

	if (m_ppBush) {
		for (int i = 0; i < m_nBush; ++i) {
			if (m_ppBush[i]) m_ppBush[i]->Release();
		}
		delete[] m_ppBush;
	}

	for (int i = 0; i < 8; ++i) if (Vents[i]) Vents[i]->Release();
	if(m_pOak) m_pOak->Release();
}

ID3D12RootSignature* GameScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[12];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtDetailAlbedoTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtDetailNormalTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 7;
	pd3dDescriptorRanges[10].BaseShaderRegister = 14; //t14: Texture2DArray
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 21; 
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[18];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 34;
	pd3dRootParameters[1].Constants.ShaderRegister = 0;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[10]; //Texture2DArray
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[16].Descriptor.ShaderRegister = 2; //DrawOptions
	pd3dRootParameters[16].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]);
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void GameScene::ReleaseUploadBuffers()
{
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
	if (m_ppPlayers) {
		for (int i = 0; i < m_nPlayers; ++i) {
			if (m_ppPlayers[i]) m_ppPlayers[i]->ReleaseUploadBuffers();
		}
	}
	if (m_ppWalls) {
		for (int i = 0; i < m_nWalls; ++i) {
			if (m_ppWalls[i]) m_ppWalls[i]->ReleaseUploadBuffers();
		}
	}
	if (m_ppBush) {
		for (int i = 0; i < m_nBush; ++i) {
			if (m_ppBush[i]) m_ppBush[i]->ReleaseUploadBuffers();
		}
	}
	if (m_pLight) m_pLight->ReleaseUploadBuffers();
	if (m_pSkybox) m_pSkybox->ReleaseUploadBuffers();

	if (m_pPianoTerrain) m_pPianoTerrain->ReleaseUploadBuffers();
	if (m_pMainTerrain) m_pMainTerrain->ReleaseUploadBuffers();
	if (m_pBroadcastTerrain) m_pBroadcastTerrain->ReleaseUploadBuffers();
	if (m_pClassroomTerrain) m_pClassroomTerrain->ReleaseUploadBuffers();
	if (m_pForestTerrain) m_pForestTerrain->ReleaseUploadBuffers();
	if (m_pCubeTerrain) m_pCubeTerrain->ReleaseUploadBuffers();
	if (m_UILogin) {
		for (int i = 0; i < m_nLogin; ++i) {
			if (m_UILogin[i]) m_UILogin[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UIRoomSelect) {
		for (int i = 0; i < m_nRoomSelect; ++i) {
			if (m_UIRoomSelect[i]) m_UIRoomSelect[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pClass) m_pClass->ReleaseUploadBuffers();
	if (m_pPiano) m_pPiano->ReleaseUploadBuffers();
	if (m_pBroadcast) m_pBroadcast->ReleaseUploadBuffers();
	if (m_pPorest) m_pPorest->ReleaseUploadBuffers();
	if (m_pLobby) m_pLobby->ReleaseUploadBuffers();
}

void GameScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

}

D3D12_GPU_DESCRIPTOR_HANDLE GameScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void GameScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);

}

void GameScene::LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);
	
	DXGI_FORMAT pdxgiRtvFormats[7] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT };

	WallShader* pShader = new WallShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 7, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };

	UINT nReads = 0, nObjectNameLength = 0;
	BYTE nStrLength = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), 14, pFile); //"<GameObjects>:"
	nReads = (UINT)::fread(&m_nWalls, sizeof(int), 1, pFile);

	m_ppWalls = new GameObject * [m_nWalls];

	GameObject* pGameObject = NULL;
	Material* pMaterial = new Material(7);
	pMaterial->m_ppTextures[0] = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/WallTex.dds", RESOURCE_TEXTURE2D, 0);
	
	CreateShaderResourceViews(pd3dDevice, pMaterial->m_ppTextures[0], 0, 3);

	pMaterial->SetShader(pShader);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	for (int i = 0; i < m_nWalls; i++)
	{
		pGameObject = new GameObject();

		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), 13, pFile); //"<GameObject>:"
		nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
		//nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
		nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile);

		pstrGameObjectName[nObjectNameLength] = '\0';
		strcpy_s(pGameObject->m_pstrFrameName, 64, pstrGameObjectName);

		Mesh* pMesh = nullptr;
		for (int j = 0; j < i; j++)
		{
			if (!strcmp(pstrGameObjectName, m_ppWalls[j]->m_pstrFrameName))
			{
				pMesh = m_ppWalls[j]->m_pMesh;
				break;
			}
		}
		if (!pMesh)
		{
			char pstrFilePath[64] = { '\0' };
			strcpy_s(pstrFilePath, 64, "Walls/");
			strcpy_s(pstrFilePath + 6, 64 - 6, pstrGameObjectName);
			strcpy_s(pstrFilePath + 6 + nObjectNameLength, 64 - 6 - nObjectNameLength, ".bin");
			pMesh = new WallMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pstrFilePath);
		}
		pGameObject->SetMesh(pMesh);

		pGameObject->renderer->SetMaterial(pMaterial);

		m_ppWalls[i] = pGameObject;
	}

	::fclose(pFile);
}

void GameScene::LoadSceneBushFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	Material* pMaterial = new Material(7);
	pMaterial->m_ppTextures[0] = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
	pMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/Bush02_d.dds", RESOURCE_TEXTURE2D, 0);

	CreateShaderResourceViews(pd3dDevice, pMaterial->m_ppTextures[0], 0, 3);

	pMaterial->SetBushShader();
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	
	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };

	UINT nReads = 0, nObjectNameLength = 0;
	BYTE nStrLength = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<GameObjects>:"
	nReads = (UINT)::fread(&m_nBush, sizeof(int), 1, pFile);

	m_ppBush = new GameObject * [m_nBush];

	GameObject* pGameObject = NULL;
	
	FILE* pMeshFile = nullptr;
	::fopen_s(&pMeshFile, "Model/bush02.bin", "rb");
	::rewind(pMeshFile);
	StandardMesh* bushMesh = new StandardMesh(pd3dDevice, pd3dCommandList);
	bushMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pMeshFile);

	for (int i = 0; i < m_nBush; i++)
	{
		pGameObject = new GameObject();

		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), 13, pFile); //"<GameObject>:"
		nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
		//nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
		nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile);


		pGameObject->SetMesh(bushMesh);

		pGameObject->renderer->SetMaterial(pMaterial);

		m_ppBush[i] = pGameObject;
		//m_ppBush[i]->SetType(-1);
	}

	::fclose(pFile);

	m_pOak = new GameObject();
	LoadedModelInfo* pOakModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Oak.bin", nullptr);
	m_pOak->SetChild(pOakModel->m_pModelRootObject, true);
	m_pOak->SetType(-1);
	//m_pOak->SetPosition(61.25, 0.7897112, -68.4);
	m_pOak->UpdateTransform(nullptr);
	if (pOakModel) delete pOakModel;
}

void GameScene::MakeVents(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pVentModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/airvent.bin", nullptr);

	for (int i = 0; i < 8; ++i) {
		Vents[i] = new Vent();
		Vents[i]->SetChild(pVentModel->m_pModelRootObject, true);
	}
	Vents[0]->SetPosition(XMFLOAT3(97.2155, 1.0061, 40.43311));
	Vents[1]->SetPosition(XMFLOAT3(97.27, 1.0061, -40.43311));
	Vents[2]->SetPosition(XMFLOAT3(20.43311, 1.0061, -77.6103));
	reinterpret_cast<Vent*>(Vents[2])->Rotate(0, 90, 0);
	reinterpret_cast<Vent*>(Vents[3])->Rotate(0, 90, 0);
	Vents[3]->SetPosition(XMFLOAT3(18.56689, 1.0061, -77.6103));
	Vents[4]->SetPosition(XMFLOAT3(-56.00388, 1.033527, -40.54385));
	Vents[5]->SetPosition(XMFLOAT3(-56.04684, 1.0061, 40.43311));
	Vents[6]->SetPosition(XMFLOAT3(35.994, 1.0061, 40.56689));
	Vents[7]->SetPosition(XMFLOAT3(35.96133, 1.0061, 23.56689));
	for (int i = 0; i < 8; ++i) {
		Vents[i]->UpdateTransform(nullptr);
	}

	if (pVentModel) delete pVentModel;
}

void GameScene::MakeDoors(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pDoorModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Future_Door_Final.bin", nullptr);
	DoorUI* doorUI = new DoorUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Login.dds");

	for (int i = 0; i < 6; ++i) {
		m_pDoors[i] = new Door();
		m_pDoors[i]->SetChild(pDoorModel->m_pModelRootObject, true);
		reinterpret_cast<Door*>(m_pDoors[i])->m_pDoorUI = doorUI;
	}
	
	m_pDoors[0]->SetPosition(XMFLOAT3(-29.73866, 0, 39.6)); 
	reinterpret_cast<Door*>(m_pDoors[0])->Rotate(0, 180, 0);
	m_pDoors[1]->SetPosition(XMFLOAT3(77.37788, 0, 39.72));
	m_pDoors[1]->UpdateTransform(nullptr);
	m_pDoors[2]->SetPosition(XMFLOAT3(23.26, 0, -39.99));
	m_pDoors[2]->UpdateTransform(nullptr);
	m_pDoors[3]->SetPosition(XMFLOAT3(-29.99397, 0, -39.71));
	m_pDoors[3]->UpdateTransform(nullptr);
	m_pDoors[4]->SetPosition(XMFLOAT3(54.99, 0, -0.4182036));
	reinterpret_cast<Door*>(m_pDoors[4])->Rotate(0, -90, 0);
	m_pDoors[5]->SetPosition(XMFLOAT3(0.18, 0, 60.25972));
	reinterpret_cast<Door*>(m_pDoors[5])->Rotate(0, -90, 0);

	if (pDoorModel) delete pDoorModel;
}

void GameScene::update(float elapsedTime, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Scene::update(elapsedTime, pd3dDevice, pd3dCommandList);

	XMFLOAT3 PlayerPos = m_pPlayer->GetPosition();

	bool IsNearDoor = false;
	for (int i = 0; i < 6; ++i) {
		m_pDoors[i]->update(elapsedTime);
		if (reinterpret_cast<Door*>(m_pDoors[i])->CheckDoor(PlayerPos)) {
			
			m_pPlayer->m_pNearDoor = m_pDoors[i];
			IsNearDoor = true;
			//printf("%d 문에 접근\n", i);
			m_pPlayer->m_door_number = i;
		}
	}
	if (IsNearDoor == false) m_pPlayer->m_pNearDoor = nullptr;
}
