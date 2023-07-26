#include "stdafx.h"
#include "Light.h"
#include "GameScene.h"
#include "Movement.h"
#include "Network.h"
#include "Input.h"
#include "Sound.h"

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
	m_sPVS[static_cast<int>(PVSROOM::CLASS_ROOM)] = std::set<PVSROOM>{ PVSROOM::CLASS_ROOM, PVSROOM::PIANO_ROOM, PVSROOM::LOBBY_ROOM, PVSROOM::FOREST };
	m_sPVS[static_cast<int>(PVSROOM::PIANO_ROOM)] = std::set<PVSROOM>{ PVSROOM::CLASS_ROOM, PVSROOM::PIANO_ROOM, PVSROOM::LOBBY_ROOM, PVSROOM::BROADCASTING_ROOM };
	m_sPVS[static_cast<int>(PVSROOM::BROADCASTING_ROOM)] = std::set<PVSROOM>{ PVSROOM::LOBBY_ROOM, PVSROOM::PIANO_ROOM, PVSROOM::CUBE_ROOM, PVSROOM::BROADCASTING_ROOM };
	m_sPVS[static_cast<int>(PVSROOM::LOBBY_ROOM)] = std::set<PVSROOM>{ PVSROOM::CLASS_ROOM, PVSROOM::PIANO_ROOM, PVSROOM::LOBBY_ROOM, PVSROOM::FOREST, PVSROOM::CUBE_ROOM, PVSROOM::BROADCASTING_ROOM };
	m_sPVS[static_cast<int>(PVSROOM::FOREST)] = std::set<PVSROOM>{ PVSROOM::CLASS_ROOM, PVSROOM::BROADCASTING_ROOM, PVSROOM::LOBBY_ROOM, PVSROOM::FOREST };
	m_sPVS[static_cast<int>(PVSROOM::CUBE_ROOM)] = std::set<PVSROOM>{ PVSROOM::LOBBY_ROOM, PVSROOM::CUBE_ROOM, PVSROOM::BROADCASTING_ROOM };

	m_pvsCamera = PVSROOM::LOBBY_ROOM;
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

void GameScene::UIrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->m_pCamera->update(pd3dCommandList);

	switch (GameState::GetInstance()->GetGameState()) {
	case LOGIN:
		for (int i = 0; i < m_nLogin; ++i)
		{
			if (i > 2) break;
			m_UILogin[i]->render(pd3dCommandList);
		}
		if (Input::GetInstance()->m_errorState != 0)
		{
			if (Input::GetInstance()->m_errorState == 1) m_UILogin[3]->render(pd3dCommandList);
			else if (Input::GetInstance()->m_errorState == 2) m_UILogin[4]->render(pd3dCommandList);

		}
		if (Input::GetInstance()->m_SuccessState) m_UILogin[5]->render(pd3dCommandList);
		break;
	case ROOM_SELECT:
		for (int i = 0; i < m_nRoomSelect; ++i) m_UIRoomSelect[i]->render(pd3dCommandList);
		break;
	case WAITING_GAME:
		for (int i = 0; i < 5; ++i) {
			m_ppPlayers[i]->SetLookAt(XMFLOAT3(0, 0, 0));
		}
		m_ppPlayers[0]->SetPosition(XMFLOAT3(6.0f, 0.0f, -5.0f));
		m_ppPlayers[1]->SetPosition(XMFLOAT3(3.0f, 0.0f, -5.0f));
		m_ppPlayers[2]->SetPosition(XMFLOAT3(-3.0f, 0.0f, -5.0f));
		m_ppPlayers[3]->SetPosition(XMFLOAT3(-6.0f, 0.0f, -5.0f));
		m_ppPlayers[4]->SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));
		m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, -3.0f));
		for (int i = 0; i < m_nWaitingRoom; ++i) m_UIWaitingRoom[i]->render(pd3dCommandList);
		break;
	case CUSTOMIZING:
		for (int i = 0; i < m_nCustomizing; ++i) m_UICustomizing[i]->render(pd3dCommandList);
		break;
	case READY_TO_GAME:
		if (GameState::GetInstance()->IsLoading()) m_UILoading[3]->render(pd3dCommandList);
		for (int i = 0; i < NUM_DOOR; ++i) {
			reinterpret_cast<Door*>(m_pDoors[i])->UIrender(pd3dCommandList);
		}
		break;
	case PLAYING_GAME:
	{
		for (int i = 0; i < NUM_DOOR; ++i) {
			reinterpret_cast<Door*>(m_pDoors[i])->UIrender(pd3dCommandList);
		}
		for (int i = 0; i < NUM_POWER; ++i) {
			reinterpret_cast<PowerSwitch*>(m_pPowers[i])->UIrender(pd3dCommandList);
		}
		for (int i = 0; i < NUM_VENT; ++i) {
			reinterpret_cast<Vent*>(Vents[i])->UIrender(pd3dCommandList);
		}
		for (int i = 0; i < NUM_ITEMBOX; ++i) {
			reinterpret_cast<ItemBox*>(m_pBoxes[i])->UIrender(pd3dCommandList);
		}
		for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
			EscapeLevers[i]->UIrender(pd3dCommandList);
		}
		reinterpret_cast<TaggersBox*>(Taggers)->UIrender(pd3dCommandList);
		for (int i = 1; i < m_nPlay; ++i) // 생명칩 프레임(0) 그리지 않음
		{
			if (i >= 1 && i <= 3) continue;
			if (i == 4 && !GameState::GetInstance()->GetChatState()) continue;
			if (i == 5 && !GameState::GetInstance()->GetMinimapState()) continue;
			m_UIPlay[i]->render(pd3dCommandList);
		}
		if (GameState::GetInstance()->GetMicState())
		{
			m_UIPlay[2]->render(pd3dCommandList);
		}
		else
		{
			m_UIPlay[3]->render(pd3dCommandList);
		}

		if (m_pPlayer->GetType() == TYPE_PLAYER)reinterpret_cast<IngameUI*>(m_UIPlay[1])->SetGuage(1.0f);
		else {
			reinterpret_cast<IngameUI*>(m_UIPlay[1])->SetGuage(-1.0f);
#if USE_NETWORK
			if (m_network->m_lifechip == true)
				reinterpret_cast<IngameUI*>(m_UIPlay[1])->SetGuage(1.0f);
#endif		
		}

		m_UIPlay[1]->render(pd3dCommandList);
		if (m_pPlayer->GetType() == TYPE_TAGGER) {
			for (int i = 0; i < 3; ++i) {
				if (m_pPlayer->GetTaggerSkill(i)) {
					reinterpret_cast<IngameUI*>(m_UITagger[i + 3])->SetGuage(1.0f);
					m_UITagger[i + 3]->render(pd3dCommandList);
				}
				else {
					reinterpret_cast<IngameUI*>(m_UITagger[i + 3])->SetGuage(-1.0f);
					m_UITagger[i + 3]->render(pd3dCommandList);
				}
			}
			for (int i = 0; i < 3; ++i) m_UITagger[i]->render(pd3dCommandList);
		}
		else {
			m_UIPlayer[0]->render(pd3dCommandList);
			int index = m_pPlayer->GetItem();
			if (index != -1) m_UIPlayer[1 + index]->render(pd3dCommandList);
		}
		int info = m_pPlayer->GetInfo();
		if (info != -1) {
			m_ppInfoUIs[info]->render(pd3dCommandList);
			m_fInfoTime += m_fElapsedTime;
			if (m_fInfoTime >= 1.5f) {
				m_fInfoTime = 0;
				m_pPlayer->SetInfo(NOINFO);
			}
		}
		break;
	}
	case ENDING_GAME:
	{
		for (int i = 0; i < 5; ++i) {
			m_ppPlayers[i]->SetLookAt(XMFLOAT3(0, 0, 0));
		}
#if USE_NETWORK
		Network& network = *Network::GetInstance();
		if (network.m_tagger_win) {
			m_UIEnding[2]->render(pd3dCommandList);
#endif
#if !USE_NETWORK
		if (0) { // TAGGER's Win
			m_UIEnding[2]->render(pd3dCommandList);
#endif
		}
		else {
			m_UIEnding[1]->render(pd3dCommandList);
		}
		m_UIEnding[0]->render(pd3dCommandList);
		break;
	}
	case INTERACTION_POWER:
	{
		if ((m_pPlayer->m_pNearInteractionObejct))
			reinterpret_cast<PowerSwitch*>(m_pPlayer->m_pNearInteractionObejct)->UIrender(pd3dCommandList);
		m_UIPlayer[0]->render(pd3dCommandList);
		int index = m_pPlayer->GetItem();
		if (index != -1) {
			m_UIPlayer[1 + index]->render(pd3dCommandList);
		}
		if (true == m_pPlayer->GetShown()) {
			m_ppAnswerUIs[0]->render(pd3dCommandList);
			if (index != -1) {
				for (int i = 1; i < 11; ++i) {
					int powerIndex{};
#if USE_NETWORK
					Network& network = *Network::GetInstance();
					powerIndex = network.m_item_to_power[index];
#endif
					reinterpret_cast<IngameUI*>(m_ppAnswerUIs[i])->SetAnswer((reinterpret_cast<PowerSwitch*>(m_pPowers[powerIndex])->GetAnswer(i - 1)));
					m_ppAnswerUIs[i]->render(pd3dCommandList);
				}
			}
		}
		m_UIPlayer[6]->render(pd3dCommandList);
		int info = m_pPlayer->GetInfo();
		if (info != -1) {
			m_ppInfoUIs[info]->render(pd3dCommandList);
			m_fInfoTime += m_fElapsedTime;
			if (m_fInfoTime >= 1.5f) {
				m_fInfoTime = 0;
				m_pPlayer->SetInfo(NOINFO);
			}
		}
		break;
	}
	}
}

void GameScene::prerender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->m_pCamera->update(pd3dCommandList);
	m_pLight->GetComponent<Light>()->SetWaitingLight(false);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);
	if(GameState::GetInstance()->GetTick()) m_pLight->GetComponent<Light>()->Updaterotate();
	XMFLOAT3 cameraPos = m_pPlayer->m_pCamera->GetPosition();
	CheckCameraPos(cameraPos);
}

void GameScene::defrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pPlayer->m_pCamera->update(pd3dCommandList);

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

	for (auto p : m_sPVS[static_cast<int>(m_pvsCamera)]) {
		m_pPVSObjects[static_cast<int>(p)]->render(pd3dCommandList);
	}

	for (int i = 0; i < NUM_VENT; ++i)
	{
		if (Vents[i]) {
			Vents[i]->UpdateTransform(nullptr);
			Vents[i]->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_DOOR; ++i)
	{
		if (m_pDoors[i]) {
			m_pDoors[i]->UpdateTransform(nullptr);
			m_pDoors[i]->render(pd3dCommandList);
		}
	}

	for (int i = 0; i < NUM_POWER; ++i) {
		if (m_pPowers[i]) {
			m_pPowers[i]->UpdateTransform(nullptr);
			reinterpret_cast<PowerSwitch*>(m_pPowers[i])->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		if (m_pBoxes[i]) {
			m_pBoxes[i]->UpdateTransform(nullptr);
			reinterpret_cast<ItemBox*>(m_pBoxes[i])->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		if (EscapeLevers[i]) {
			EscapeLevers[i]->UpdateTransform(nullptr);
			EscapeLevers[i]->render(pd3dCommandList);
		}
	}
	if (Taggers) {
		Taggers->UpdateTransform(nullptr);
		reinterpret_cast<TaggersBox*>(Taggers)->render(pd3dCommandList);
	}
	if (m_sPVS[static_cast<int>(m_pvsCamera)].count(PVSROOM::FOREST) != 0) {
		m_pOak->render(pd3dCommandList);
		for (int i = 0; i < m_nBush; ++i)
		{
			if (m_ppBush[i]) m_ppBush[i]->render(pd3dCommandList);
		}
	}
	//std::cout << "x" << m_pPlayer->GetPosition().x << std::endl;
	//std::cout << "y" << m_pPlayer->GetPosition().y << std::endl;
	//std::cout << "z" << m_pPlayer->GetPosition().z << std::endl;
}

void GameScene::WaitingRoomrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->m_pCamera->update(pd3dCommandList);
	m_pLight->GetComponent<Light>()->SetWaitingLight(true);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);
	Scene::render(pd3dCommandList);
}

void GameScene::Endingrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	m_pPlayer->SetPosition(XMFLOAT3(0.0f, -2.0f, -3.0f));
	m_pPlayer->m_pCamera->update(pd3dCommandList);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);
		
#if USE_NETWORK
	Network& network = *Network::GetInstance();
	if (network.m_tagger_win) {
#endif
#if !USE_NETWORK
	if (0) { // TAGGER's Win
#endif
		m_pPlayer->OnPrepareRender();
		m_pPlayer->Animate(m_fElapsedTime, m_pPlayer->PlayerNum);
		if (m_pPlayer->GetType() == TYPE_TAGGER) {
			m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 10);
			m_pPlayer->render(pd3dCommandList);
		}
		else {
			for (int i = 0; i < 5; ++i) {
				m_ppPlayers[i]->OnPrepareRender();
				m_ppPlayers[i]->Animate(m_fElapsedTime, m_ppPlayers[i]->PlayerNum);
				if (m_ppPlayers[i]->GetType() == TYPE_TAGGER) {
					m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 10);
					m_ppPlayers[i]->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
					m_ppPlayers[i]->render(pd3dCommandList);
				}
			}
		}
	}
	else {
		int EscapeNum = 0;
		m_pPlayer->OnPrepareRender();
		m_pPlayer->Animate(m_fElapsedTime, m_pPlayer->PlayerNum);
		if (m_pPlayer->GetType() == TYPE_ESCAPE_PLAYER) {
			m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 10);
			m_pPlayer->render(pd3dCommandList);
		}
		for (int i = 0; i < 5; ++i) {
			if (m_ppPlayers[i]->GetType() == TYPE_ESCAPE_PLAYER) {
				EscapeNum++;
			}
		}
		float StartXPos = (EscapeNum - 1) * -1.5f;
		for (int i = 0; i < 5; ++i) {
			if (m_ppPlayers[i]->GetType() == TYPE_ESCAPE_PLAYER) {
				m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 10);
				m_ppPlayers[i]->SetPosition(XMFLOAT3(StartXPos, -2.0f, -5.0f));
				m_ppPlayers[i]->OnPrepareRender();
				m_ppPlayers[i]->Animate(m_fElapsedTime, m_ppPlayers[i]->PlayerNum);
				m_ppPlayers[i]->render(pd3dCommandList);
				StartXPos += 3.0f;
			}
		}
	}
	m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, -3.0f));
}

void GameScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 260);

	Material::PrepareUIShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_nLoading = 4;
	m_UILoading = new GameObject * [m_nLoading];
	m_UILoading[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/LoadingPro.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UILoading[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Progress_Background.dds", -0.55f, -0.65f, 0.8f, 0.14f);
	m_UILoading[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Progress_Bar.dds", -0.55f, -0.65f, 0.8f, 0.14f);
	m_UILoading[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Loading.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetUIType(PROGRESS_BAR_UI);
	
	m_pLight = new GameObject();
	m_pLight->AddComponent<Light>();
	m_pLight->start(pd3dDevice, pd3dCommandList);
	m_pLights = reinterpret_cast<Light*>(m_pLight)->GetLights();

	m_nPlayers = 5;
	m_ppPlayers = new Player * [m_nPlayers];
	for (int i = 0; i < m_nPlayers; ++i) {
		m_ppPlayers[i] = new Player();
	}

	m_pPlayer = new Player();
	m_pPlayer->DeleteComponent<OtherPlayerCamera>();
	m_pPlayer->AddComponent<ThirdPersonCamera>();
	m_pPlayer->m_pCamera = m_pPlayer->GetComponent<ThirdPersonCamera>();
	m_pPlayer->m_pCamera->start(pd3dDevice, pd3dCommandList);
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
	if (m_pPVSObjects)
		for (int i = 0; i < 6; ++i) {
			if (m_pPVSObjects[i]) m_pPVSObjects[i]->Release();
		}
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

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[13];

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

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = MAX_DEPTH_TEXTURES;
	pd3dDescriptorRanges[12].BaseShaderRegister = 22; //Depth Buffer
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[21];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 34; //object
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

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[18].Constants.Num32BitValues = 2;
	pd3dRootParameters[18].Constants.ShaderRegister = 3;
	pd3dRootParameters[18].Constants.RegisterSpace = 0;
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[19].Descriptor.ShaderRegister = 6; //ToLight//
	pd3dRootParameters[19].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[20].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[20].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; //Depth Buffer//
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

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

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0.0f;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].MipLODBias = 0.0f;
	pd3dSamplerDescs[3].MaxAnisotropy = 1;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

void GameScene::Depthrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pSkybox->Depthrender(pd3dCommandList);

	//m_pCeilling->Depthrender(pd3dCommandList);

	m_pMainTerrain->Depthrender(pd3dCommandList);
	m_pPianoTerrain->Depthrender(pd3dCommandList);
	m_pBroadcastTerrain->Depthrender(pd3dCommandList);
	m_pCubeTerrain->Depthrender(pd3dCommandList);
	m_pForestTerrain->Depthrender(pd3dCommandList);
	m_pClassroomTerrain->Depthrender(pd3dCommandList);

	for (int i = 0; i < m_nWalls; ++i)
	{
		if (m_ppWalls[i]) m_ppWalls[i]->Depthrender(pd3dCommandList);
	}

	Scene::Depthrender(pd3dCommandList);

	for (auto p : m_sPVS[static_cast<int>(m_pvsCamera)]) {
		m_pPVSObjects[static_cast<int>(p)]->Depthrender(pd3dCommandList);
	}

	for (int i = 0; i < NUM_VENT; ++i)
	{
		if (Vents[i]) {
			Vents[i]->UpdateTransform(nullptr);
			Vents[i]->Depthrender(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_DOOR; ++i)
	{
		if (m_pDoors[i]) {
			m_pDoors[i]->UpdateTransform(nullptr);
			m_pDoors[i]->Depthrender(pd3dCommandList);
		}
	}

	for (int i = 0; i < NUM_POWER; ++i) {
		if (m_pPowers[i]) {
			m_pPowers[i]->UpdateTransform(nullptr);
			reinterpret_cast<PowerSwitch*>(m_pPowers[i])->Depthrender(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		if (m_pBoxes[i]) {
			m_pBoxes[i]->UpdateTransform(nullptr);
			reinterpret_cast<ItemBox*>(m_pBoxes[i])->Depthrender(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		if (EscapeLevers[i]) {
			EscapeLevers[i]->UpdateTransform(nullptr);
			EscapeLevers[i]->Depthrender(pd3dCommandList);
		}
	}
	if (Taggers) {
		Taggers->UpdateTransform(nullptr);
		reinterpret_cast<TaggersBox*>(Taggers)->Depthrender(pd3dCommandList);
	}
	if (m_sPVS[static_cast<int>(m_pvsCamera)].count(PVSROOM::FOREST) != 0) {
		m_pOak->Depthrender(pd3dCommandList);
		for (int i = 0; i < m_nBush; ++i)
		{
			if (m_ppBush[i]) m_ppBush[i]->Depthrender(pd3dCommandList);
		}
	}
}

void GameScene::ReleaseUploadBuffers()
{
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	if (m_ppPlayers) {
		for (int i = 0; i < m_nPlayers; ++i) {
			if (m_ppPlayers[i]) m_ppPlayers[i]->ReleaseUploadBuffers();
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
	if (m_UIWaitingRoom) {
		for (int i = 0; i < m_nWaitingRoom; ++i) {
			if (m_UIWaitingRoom[i]) m_UIWaitingRoom[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UICustomizing) {
		for (int i = 0; i < m_nCustomizing; ++i) {
			if (m_UICustomizing[i]) m_UICustomizing[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UIEnding) {
		for (int i = 0; i < m_Ending; ++i) {
			if (m_UIEnding[i]) m_UIEnding[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UIPlay) {
		for (int i = 0; i < m_nPlay; ++i) {
			if (m_UIPlay[i]) m_UIPlay[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UIPlayer) {
		for (int i = 0; i < m_nPlayPlayer; ++i) {
			if (m_UIPlayer[i]) m_UIPlayer[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UITagger) {
		for (int i = 0; i < m_nPlayTagger; ++i) {
			if (m_UITagger[i]) m_UITagger[i]->ReleaseUploadBuffers();
		}
	}
	if (m_UILoading) {
		for (int i = 0; i < m_nLoading; ++i) {
			if (m_UILoading[i]) m_UILoading[i]->ReleaseUploadBuffers();
		}
	}
	if (m_pCeilling) m_pCeilling->ReleaseUploadBuffers();

	if (m_ppWalls) {
		for (int i = 0; i < m_nWalls; ++i) {
			if (m_ppWalls[i]) m_ppWalls[i]->ReleaseUploadBuffers();
		}
	}

	for (int i = 0; i < 6; ++i) {
		if (m_pPVSObjects[i]) m_pPVSObjects[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < NUM_VENT; ++i) {
		if (Vents[i]) Vents[i]->ReleaseUploadBuffers();
	}
	if (m_ppBush) {
		for (int i = 0; i < m_nBush; ++i) {
			if (m_ppBush[i]) m_ppBush[i]->ReleaseUploadBuffers();
		}
	}
	if (m_pOak) m_pOak->ReleaseUploadBuffers();

	for (int i = 0; i < NUM_DOOR; ++i) {
		if (m_pDoors[i]) m_pDoors[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		if (m_pBoxes[i]) m_pBoxes[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < NUM_POWER; ++i) {
		if (m_pPowers[i]) m_pPowers[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < 6; ++i) {
		if (Items[i]) Items[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < m_nObjectsUIs; ++i) {
		if (m_ppObjectsUIs[i]) m_ppObjectsUIs[i]->ReleaseUploadBuffers();
	}
	if (Taggers) Taggers->ReleaseUploadBuffers();
	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		if (EscapeLevers[i]) EscapeLevers[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < m_nAnswerUI; ++i) {
		if (m_ppAnswerUIs[i]) m_ppAnswerUIs[i]->ReleaseUploadBuffers();
	}
	for (int i = 0; i < m_nObjectsUIs; ++i) {
		if (m_ppObjectsUIs[i]) m_ppObjectsUIs[i]->ReleaseUploadBuffers();
	}
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

void GameScene::Powerrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nWalls; ++i)
	{
		if (m_ppWalls[i]) m_ppWalls[i]->render(pd3dCommandList);
	}
	if ((m_pPlayer->m_pNearInteractionObejct))
		m_pPlayer->m_pNearInteractionObejct->render(pd3dCommandList);
}

void GameScene::Loadingrender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	if (m_pPlayer)
		if (m_pPlayer->m_pCamera)
			m_pPlayer->m_pCamera->update(pd3dCommandList);
	for (int i = 0; i < m_nLoading - 1; ++i) {
		reinterpret_cast<IngameUI*>(m_UILoading[i])->render(pd3dCommandList);
	}
}

void GameScene::SpectatorPrerender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	int index = m_pPlayer->SpectatorPlayerIndex;

	bool findNew = false;
	if (m_ppPlayers[index]->GetType() == TYPE_ESCAPE_PLAYER || m_ppPlayers[index]->GetType() == TYPE_TAGGER) {
		for (int i = index + 1; i < 5; ++i) {
			if (m_ppPlayers[index]->GetType() != TYPE_ESCAPE_PLAYER && m_ppPlayers[index]->GetType() != TYPE_TAGGER) {
				findNew = true;
				m_nSpectator = i;
				m_pPlayer->ChangeSpectator(i);
				break;
			}
		}
		if (findNew == false) {
			for (int i = 0; i < index; ++i) {
				if (m_ppPlayers[index]->GetType() != TYPE_ESCAPE_PLAYER && m_ppPlayers[index]->GetType() != TYPE_TAGGER) {
					findNew = true;
					m_nSpectator = i;
					m_pPlayer->ChangeSpectator(i);
					break;
				}
			}
		}
	}
	else {
		m_nSpectator = index;
	}
	m_ppPlayers[m_nSpectator]->m_pCamera->update(pd3dCommandList);
	XMFLOAT3 cameraPos = m_ppPlayers[m_nSpectator]->m_pCamera->GetPosition();
	CheckCameraPos(cameraPos);

	m_pLight->GetComponent<Light>()->SetWaitingLight(false);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);
	if (GameState::GetInstance()->GetTick())m_pLight->GetComponent<Light>()->Updaterotate();
}

void GameScene::Spectatorrender(ID3D12GraphicsCommandList* pd3dCommandList)
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

	for (int i = 0; i < 5; ++i) {
		if (m_ppPlayers[i]->GetType() != TYPE_ESCAPE_PLAYER) {
			m_ppPlayers[i]->OnPrepareRender();
			m_ppPlayers[i]->Animate(m_fElapsedTime, m_ppPlayers[i]->PlayerNum);
			m_ppPlayers[i]->render(pd3dCommandList);
		}
			
	}

	for (auto p : m_sPVS[static_cast<int>(m_pvsCamera)]) {
		m_pPVSObjects[static_cast<int>(p)]->render(pd3dCommandList);
	}

	for (int i = 0; i < NUM_VENT; ++i)
	{
		if (Vents[i]) {
			Vents[i]->UpdateTransform(nullptr);
			Vents[i]->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_DOOR; ++i)
	{
		if (m_pDoors[i]) {
			m_pDoors[i]->UpdateTransform(nullptr);
			m_pDoors[i]->render(pd3dCommandList);
		}
	}

	for (int i = 0; i < NUM_POWER; ++i) {
		if (m_pPowers[i]) {
			m_pPowers[i]->UpdateTransform(nullptr);
			reinterpret_cast<PowerSwitch*>(m_pPowers[i])->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		if (m_pBoxes[i]) {
			m_pBoxes[i]->UpdateTransform(nullptr);
			reinterpret_cast<ItemBox*>(m_pBoxes[i])->render(pd3dCommandList);
		}
	}
	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		if (EscapeLevers[i]) {
			EscapeLevers[i]->UpdateTransform(nullptr);
			EscapeLevers[i]->render(pd3dCommandList);
		}
	}
	if (Taggers) {
		Taggers->UpdateTransform(nullptr);
		reinterpret_cast<TaggersBox*>(Taggers)->render(pd3dCommandList);
	}
	if (m_sPVS[static_cast<int>(m_pvsCamera)].count(PVSROOM::FOREST) != 0) {
		m_pOak->render(pd3dCommandList);
		for (int i = 0; i < m_nBush; ++i)
		{
			if (m_ppBush[i]) m_ppBush[i]->render(pd3dCommandList);
		}
	}
}

void GameScene::LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);
	
	DXGI_FORMAT pdxgiRtvFormats[6] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32_FLOAT };

	WallShader* pShader = new WallShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 6, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
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

	for (int i = 0; i < NUM_VENT; ++i) {
		Vents[i] = new Vent();
		Vents[i]->SetChild(pVentModel->m_pModelRootObject, true);
		reinterpret_cast<Vent*>(Vents[i])->SetUI(0, m_ppObjectsUIs[0]);
		reinterpret_cast<Vent*>(Vents[i])->SetUI(1, m_ppObjectsUIs[5]);
	}
	Vents[0]->SetPosition(XMFLOAT3(97.2155f, 1.0061f, 40.43311f));
	reinterpret_cast<Vent*>(Vents[0])->SetOpenPos(XMFLOAT3(97.2155f, 2.0061f, 41.43311f));
	reinterpret_cast<Vent*>(Vents[0])->SetRotation(DEGREE0);
	Vents[1]->SetPosition(XMFLOAT3(97.27f, 1.0061f, -40.43311f));
	reinterpret_cast<Vent*>(Vents[1])->SetOpenPos(XMFLOAT3(97.27f, 2.0061f, -41.43311f));
	reinterpret_cast<Vent*>(Vents[1])->SetRotation(DEGREE180);
	Vents[2]->SetPosition(XMFLOAT3(20.43311f, 1.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[2])->SetOpenPos(XMFLOAT3(21.43311f, 2.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[2])->SetRotation(DEGREE90);
	Vents[3]->SetPosition(XMFLOAT3(18.56689f, 1.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[3])->SetOpenPos(XMFLOAT3(17.56689f, 2.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[3])->SetRotation(DEGREE270);
	Vents[4]->SetPosition(XMFLOAT3(-56.00388f, 1.0061f, -40.54385f));
	reinterpret_cast<Vent*>(Vents[4])->SetOpenPos(XMFLOAT3(-56.00388f, 2.0061f, -41.54385f));
	reinterpret_cast<Vent*>(Vents[4])->SetRotation(DEGREE180);
	Vents[5]->SetPosition(XMFLOAT3(-56.04684f, 1.0061f, 40.43311f));
	reinterpret_cast<Vent*>(Vents[5])->SetOpenPos(XMFLOAT3(-56.04684f, 2.0061f, 41.43311f));
	reinterpret_cast<Vent*>(Vents[5])->SetRotation(DEGREE0);
	Vents[6]->SetPosition(XMFLOAT3(35.994f, 1.0061f, 40.56689f));
	reinterpret_cast<Vent*>(Vents[6])->SetOpenPos(XMFLOAT3(35.994f, 2.0061f, 41.56689f));
	reinterpret_cast<Vent*>(Vents[6])->SetRotation(DEGREE0);
	Vents[7]->SetPosition(XMFLOAT3(35.96133f, 1.0061f, 23.56689f));
	reinterpret_cast<Vent*>(Vents[7])->SetOpenPos(XMFLOAT3(35.96133f, 2.0061f, 22.56689f));
	reinterpret_cast<Vent*>(Vents[7])->SetRotation(DEGREE180);

	/*Vents[0]->SetPosition(XMFLOAT3(97.2155f, 1.0061f, 40.43311f));
	reinterpret_cast<Vent*>(Vents[0])->SetOpenPos(XMFLOAT3(98.94085f, 1.0061f, 42.29158f));
	reinterpret_cast<Vent*>(Vents[0])->SetRotation(DEGREE0);
	Vents[1]->SetPosition(XMFLOAT3(97.27f, 1.0061f, -40.43311f));
	reinterpret_cast<Vent*>(Vents[1])->SetOpenPos(XMFLOAT3(95.5352f, 1.0061f, -42.2919f));
	reinterpret_cast<Vent*>(Vents[1])->SetRotation(DEGREE0);
	Vents[2]->SetPosition(XMFLOAT3(20.43311f, 1.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[2])->SetOpenPos(XMFLOAT3(22.29154f, 1.0061f, -75.88629f));
	reinterpret_cast<Vent*>(Vents[2])->SetRotation(DEGREE90);
	Vents[3]->SetPosition(XMFLOAT3(18.56689f, 1.0061f, -77.6103f));
	reinterpret_cast<Vent*>(Vents[3])->SetOpenPos(XMFLOAT3(16.70864f, 1.0061f, -79.3296f));
	reinterpret_cast<Vent*>(Vents[3])->SetRotation(DEGREE90);
	Vents[4]->SetPosition(XMFLOAT3(-56.00388f, 1.033527f, -40.54385f));
	reinterpret_cast<Vent*>(Vents[4])->SetOpenPos(XMFLOAT3(-57.66415f, 1.0061f, -42.1952f));
	reinterpret_cast<Vent*>(Vents[4])->SetRotation(DEGREE0);
	Vents[5]->SetPosition(XMFLOAT3(-56.04684f, 1.0061f, 40.43311f));
	reinterpret_cast<Vent*>(Vents[5])->SetOpenPos(XMFLOAT3(-54.31968f, 1.0061f, 42.29163f));
	reinterpret_cast<Vent*>(Vents[5])->SetRotation(DEGREE0);
	Vents[6]->SetPosition(XMFLOAT3(35.994f, 1.0061f, 40.56689f));
	reinterpret_cast<Vent*>(Vents[6])->SetOpenPos(XMFLOAT3(37.63297f, 1.0061f, 42.2901f));
	reinterpret_cast<Vent*>(Vents[6])->SetRotation(DEGREE0);
	Vents[7]->SetPosition(XMFLOAT3(35.96133f, 1.0061f, 23.56689f));
	reinterpret_cast<Vent*>(Vents[7])->SetOpenPos(XMFLOAT3(34.23397f, 1.0061f, 21.70837f));
	reinterpret_cast<Vent*>(Vents[7])->SetRotation(DEGREE0);
	*/
	for (int i = 0; i < NUM_VENT; ++i) {
		Vents[i]->UpdateTransform(nullptr);
	}

	if (pVentModel) delete pVentModel;
}

void GameScene::MakeDoors(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pDoorModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Future_Door_Final.bin", nullptr);
	
	for (int i = 0; i < NUM_DOOR; ++i) {
		m_pDoors[i] = new Door();
		m_pDoors[i]->SetChild(pDoorModel->m_pModelRootObject, true);
		reinterpret_cast<Door*>(m_pDoors[i])->SetUI(0, m_ppObjectsUIs[0]);
		reinterpret_cast<Door*>(m_pDoors[i])->SetUI(1, m_ppObjectsUIs[1]);
		reinterpret_cast<Door*>(m_pDoors[i])->SetUI(2, m_ppObjectsUIs[5]);
	}
	
	m_pDoors[0]->SetPosition(XMFLOAT3(-29.73866f, 0.0f, 39.6f)); 
	reinterpret_cast<Door*>(m_pDoors[0])->SetRotation(DEGREE180);
	m_pDoors[1]->SetPosition(XMFLOAT3(77.37788f, 0.0f, 39.72f));
	reinterpret_cast<Door*>(m_pDoors[1])->SetRotation(DEGREE0);
	m_pDoors[1]->UpdateTransform(nullptr);
	m_pDoors[2]->SetPosition(XMFLOAT3(23.26f, 0.0f, -39.99f));
	reinterpret_cast<Door*>(m_pDoors[2])->SetRotation(DEGREE0);
	m_pDoors[2]->UpdateTransform(nullptr);

	m_pDoors[3]->SetPosition(XMFLOAT3(-29.99397f, 0.0f, -39.71f));
	reinterpret_cast<Door*>(m_pDoors[3])->SetRotation(DEGREE0);
	m_pDoors[3]->UpdateTransform(nullptr);

	m_pDoors[4]->SetPosition(XMFLOAT3(54.99f, 0.0f, -0.4182036f));
	reinterpret_cast<Door*>(m_pDoors[4])->SetRotation(DEGREE270);

	m_pDoors[5]->SetPosition(XMFLOAT3(0.18f, 0.0f, 60.25972f));
	reinterpret_cast<Door*>(m_pDoors[5])->SetRotation(DEGREE270);

	if (pDoorModel) delete pDoorModel;
}

void GameScene::MakePowers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pElecModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Power.bin", nullptr);
	
	pElecModel->m_pModelRootObject->SetScale(0.5, 0.5, 0.5);
	for (int i = 0; i < NUM_POWER; ++i) {
		m_pPowers[i] = new PowerSwitch();
		m_pPowers[i]->SetChild(pElecModel->m_pModelRootObject, true);
		m_pPowers[i]->Init();
		m_pPowers[i]->SetUI(0, m_ppObjectsUIs[0]);
		m_pPowers[i]->SetUI(1, m_ppObjectsUIs[1]);
		m_pPowers[i]->SetUI(2, m_ppObjectsUIs[3]);
		m_pPowers[i]->SetUI(3, m_ppObjectsUIs[4]);
		m_pPowers[i]->SetUI(4, m_ppObjectsUIs[5]);
		//m_pPowers[i]->UpdateTransform(nullptr);
	}

	m_pPowers[0]->SetPosition(XMFLOAT3(-0.7033535f, 1.5f, 76.76f)); // piano
	m_pPowers[0]->SetRotation(DEGREE270);
	m_pPowers[1]->SetPosition(XMFLOAT3(-54.11389f, 1.5f, -66.95f)); // classroom
	m_pPowers[1]->SetRotation(DEGREE270);
	m_pPowers[2]->SetPosition(XMFLOAT3(60.87f, 1.5f, -70.0f)); // porest
	m_pPowers[2]->SetRotation(DEGREE180);
	m_pPowers[3]->SetPosition(XMFLOAT3(67.6f, 1.5f, 40.6322f)); // broadcastingroom
	m_pPowers[3]->SetRotation(DEGREE0);
	m_pPowers[4]->SetPosition(XMFLOAT3(65.231f, 1.5f, -27.5f)); // maze
	m_pPowers[4]->SetRotation(DEGREE270);

	if (pElecModel) delete pElecModel;
}

void GameScene::update(float elapsedTime, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Scene::update(elapsedTime, pd3dDevice, pd3dCommandList);

	XMFLOAT3 PlayerPos = m_pPlayer->GetPosition();

	bool IsNearDoor = false;
	bool IsNearInteractionObject = false;
	bool IsNearVent = false;
	bool IsNearItembox = false;
	bool IsNearTaggers = false;
	bool IsNearEsacpeLever = false;
	for (int i = 0; i < NUM_DOOR; ++i) {
		m_pDoors[i]->update(elapsedTime);
		if (reinterpret_cast<Door*>(m_pDoors[i])->IsPlayerNear(PlayerPos)) {
			
			m_pPlayer->m_pNearDoor = m_pDoors[i];
			IsNearDoor = true;
			//printf("%d 문에 접근\n", i);
			m_pPlayer->m_door_number = i;
		}
	}
	for (int i = 0; i < NUM_POWER; ++i) {
		m_pPowers[i]->update(elapsedTime);
		if (m_pPowers[i]->IsPlayerNear(PlayerPos)) {
			m_pPlayer->m_pNearInteractionObejct = m_pPowers[i];
			IsNearInteractionObject = true;

			m_pPlayer->m_power_number = i;
		}
	}
	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		m_pBoxes[i]->update(elapsedTime);
		if (m_pBoxes[i]->IsPlayerNear(PlayerPos)) {
			m_pPlayer->m_pNearItembox = reinterpret_cast<GameObject*>(m_pBoxes[i]);
			IsNearItembox = true;

			m_pPlayer->m_itembox_number = i;
		}
	}
	for (int i = 0; i < NUM_VENT; ++i) {
		Vents[i]->update(elapsedTime);
		if (reinterpret_cast<Vent*>(Vents[i])->IsPlayerNear(PlayerPos)) {
			m_pPlayer->m_pNearVent = Vents[i];
			IsNearVent = true;

			m_pPlayer->m_vent_number = i;
		}
	}
	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		EscapeLevers[i]->update(elapsedTime);
		if (reinterpret_cast<EscapeObject*>(EscapeLevers[i])->IsPlayerNear(PlayerPos)) {
			m_pPlayer->m_pNearEscape = EscapeLevers[i];
			IsNearEsacpeLever = true;

			m_pPlayer->m_escape_number = i;
		}
	}
	Taggers->update(elapsedTime);
	if (reinterpret_cast<TaggersBox*>(Taggers)->IsPlayerNear(PlayerPos)) {
		m_pPlayer->m_pNearTaggers = Taggers;
		IsNearTaggers = true;
	}
	if (IsNearDoor == false) m_pPlayer->m_pNearDoor = nullptr;
	if (IsNearInteractionObject == false) m_pPlayer->m_pNearInteractionObejct = nullptr;
	if (IsNearVent == false) m_pPlayer->m_pNearVent = nullptr;
	if (IsNearItembox == false) m_pPlayer->m_pNearItembox = nullptr;
	if (IsNearTaggers == false) m_pPlayer->m_pNearTaggers = nullptr;
	if (IsNearEsacpeLever == false) m_pPlayer->m_pNearEscape = nullptr;

	Sound& sound = *Sound::GetInstance();
	for (int i = 0; i < 5; ++i) {
		if (m_ppPlayers[i]) {
			XMFLOAT3 pos = m_ppPlayers[i]->GetPosition();
			sound.SetOtherPlayersPos(i, pos);
		}
	}
	sound.SetListenerPos(m_pPlayer->GetPosition(), m_pPlayer->GetLookVector(), m_pPlayer->GetUpVector());
	sound.Update(elapsedTime);
}

bool InArea(int startX, int startZ, int width, int length, float x, float z)
{
	float minx, minz, maxx, maxz;
	minx = startX - float(width / 2);
	minz = startZ - float(length / 2);
	maxx = startX + float(width / 2);
	maxz = startZ + float(length / 2);

	if (x > maxx) return false;
	if (x < minx) return false;
	if (z > maxz) return false;
	if (z < minz) return false;
	return true;
}

void GameScene::CheckCameraPos(const XMFLOAT3 camera)
{
	float x = camera.x; float z = camera.z;

	if (InArea(0, 0, 120, 80, x, z)) { 
		m_pvsCamera = PVSROOM::LOBBY_ROOM;
		return;
	}
	else if (InArea(-30, 60, 60, 40, x, z)) {
		m_pvsCamera = PVSROOM::PIANO_ROOM;
		return;
	}
	else if (InArea(50, 60, 100, 40, x, z)) {
		m_pvsCamera = PVSROOM::BROADCASTING_ROOM;
		return;
	}
	else if (InArea(80, 0, 40, 80, x, z)) {
		m_pvsCamera = PVSROOM::CUBE_ROOM;
		return;
	}
	else if (InArea(60, -60, 80, 40, x, z)) {
		m_pvsCamera = PVSROOM::FOREST;
		return;
	}
	else if (InArea(-20, -60, 80, 40, x, z)) {
		m_pvsCamera = PVSROOM::CLASS_ROOM;
		return;
	}
}

void GameScene::MakeBoxes(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pBoxModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/AmmoBox.bin", nullptr);
	LoadedModelInfo* pDrillModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Drill.bin", nullptr);
	LoadedModelInfo* pHammerModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Hammer_01.bin", nullptr);
	LoadedModelInfo* pPliersModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Pliers.bin", nullptr);
	LoadedModelInfo* pDriverModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Screwdriver_Cross.bin", nullptr);
	LoadedModelInfo* pWrenchModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Wrench_Combination.bin", nullptr);
	LoadedModelInfo* pChipModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Chip.bin", nullptr);

	Items[0] = new GameObject();
	Items[0]->SetChild(pHammerModel->m_pModelRootObject, true);
	Items[1] = new GameObject();
	Items[1]->SetChild(pDrillModel->m_pModelRootObject, true);
	Items[2] = new GameObject();
	Items[2]->SetChild(pWrenchModel->m_pModelRootObject, true);
	Items[3] = new GameObject();
	Items[3]->SetChild(pPliersModel->m_pModelRootObject, true);
	Items[4] = new GameObject();
	Items[4]->SetChild(pDriverModel->m_pModelRootObject, true);
	Items[5] = new GameObject();
	Items[5]->SetChild(pChipModel->m_pModelRootObject, true);

	for (int i = 0; i < NUM_ITEMBOX; ++i) {
		m_pBoxes[i] = new ItemBox();
		m_pBoxes[i]->SetChild(pBoxModel->m_pModelRootObject, true);
		m_pBoxes[i]->SetUI(0,m_ppObjectsUIs[0]);
		m_pBoxes[i]->SetUI(1, m_ppObjectsUIs[1]);
		m_pBoxes[i]->SetUI(2, m_ppObjectsUIs[2]); 
		m_pBoxes[i]->SetUI(3, m_ppObjectsUIs[5]);
		for (int j = 0; j < 6; ++j) {
			if (Items[j]) {
				m_pBoxes[i]->InitItems(j, Items[j]);
			}
		}
	}
	
	m_pBoxes[0]->SetPosition(2.29f, 0.0f, 70.7f); // InBroadcastingRoom
	m_pBoxes[0]->SetRotation(DEGREE90);
	m_pBoxes[1]->SetPosition(36.91f, 0.0f, 64.079f);
	m_pBoxes[1]->SetRotation(DEGREE180);
	m_pBoxes[2]->SetPosition(87.3f, 0.0f, 77.7f);
	m_pBoxes[2]->SetRotation(DEGREE180);

	m_pBoxes[3]->SetPosition(66.924f, 0.0f, 11.678f); // InCubeRoom
	m_pBoxes[3]->SetRotation(DEGREE270);
	m_pBoxes[4]->SetPosition(63.465f, 0.0f, 26.12f);
	m_pBoxes[4]->SetRotation(DEGREE90);
	m_pBoxes[5]->SetPosition(85.75f, 0.0f, 26.9f);
	m_pBoxes[5]->SetRotation(DEGREE270);

	m_pBoxes[6]->SetPosition(49.261f, 0.1f, -65.91f); // InPorset
	m_pBoxes[6]->SetRotation(DEGREE180);
	m_pBoxes[7]->SetPosition(84.502f, 0.1f, -50.275f);
	m_pBoxes[7]->SetRotation(DEGREE180);
	m_pBoxes[8]->SetPosition(61.45f, 0.0f, -65.87f);
	m_pBoxes[8]->SetRotation(DEGREE0);

	m_pBoxes[9]->SetPosition(9.2f, 0.0f, -41.4f); // InClassRoom
	m_pBoxes[9]->SetRotation(DEGREE180);
	m_pBoxes[10]->SetPosition(9.4f, 0.0f, -78.42f);
	m_pBoxes[10]->SetRotation(DEGREE0);
	m_pBoxes[11]->SetPosition(-18.2f, 0.0f, -60.8f);
	m_pBoxes[11]->SetRotation(DEGREE270);
	m_pBoxes[12]->SetPosition(-56.89f, 0.0f, -59.96f);
	m_pBoxes[12]->SetRotation(DEGREE90);

	m_pBoxes[13]->SetPosition(-56.8f, 0.0f, 60.05f); // InPianoRoom
	m_pBoxes[13]->SetRotation(DEGREE90);
	m_pBoxes[14]->SetPosition(-16.757f, 0.0f, 59.201f);
	m_pBoxes[14]->SetRotation(DEGREE0);
	m_pBoxes[15]->SetPosition(-38.81f, 0.0f, 74.12f);
	m_pBoxes[15]->SetRotation(DEGREE0);

	m_pBoxes[16]->SetPosition(21.59f, 0.0f, 32.4f); // InLobby
	m_pBoxes[16]->SetRotation(DEGREE180);
	m_pBoxes[17]->SetPosition(37.7f, 0.0f, -0.4f);
	m_pBoxes[17]->SetRotation(DEGREE270);
	m_pBoxes[17]->SetItem(GAME_ITEM::ITEM_HAMMER);
	m_pBoxes[18]->SetPosition(-34.2f, 0.0f, 0.5f);
	m_pBoxes[18]->SetRotation(DEGREE270);
	m_pBoxes[18]->SetItem(GAME_ITEM::ITEM_LIFECHIP);
	m_pBoxes[19]->SetPosition(-3.7f, 0.0f, 16.734f);
	m_pBoxes[19]->SetRotation(DEGREE0);
	m_pBoxes[19]->SetItem(GAME_ITEM::ITEM_DRILL);
	
	if (pBoxModel) delete pBoxModel;

	if (pDrillModel) delete pDrillModel;
	if (pHammerModel) delete pHammerModel;
	if (pPliersModel) delete pPliersModel;
	if (pDriverModel) delete pDriverModel;
	if (pWrenchModel) delete pWrenchModel;
	if (pChipModel) delete pChipModel;
}

void GameScene::MakeTaggers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pAltarModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Altar.bin", nullptr);

	Taggers = new TaggersBox();
	Taggers->SetChild(pAltarModel->m_pModelRootObject, true);
	Taggers->SetUI(0, m_ppObjectsUIs[8]);
	Taggers->SetUI(1, m_ppObjectsUIs[6]);
	Taggers->SetUI(2, m_ppObjectsUIs[5]);
	Taggers->SetPosition(-3.2f, 0.93f, -0.34f);
	reinterpret_cast<TaggersBox*>(Taggers)->Init();

	if (pAltarModel) delete pAltarModel;
}

void GameScene::MakeEscapeLevers(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	LoadedModelInfo* pLeverModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/EscapeLever.bin", nullptr);

	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i) {
		EscapeLevers[i] = new EscapeObject();
		EscapeLevers[i]->SetChild(pLeverModel->m_pModelRootObject, true);
		EscapeLevers[i]->SetUI(0, m_ppObjectsUIs[7]);
		EscapeLevers[i]->SetUI(1, m_ppObjectsUIs[5]);
		EscapeLevers[i]->Init();
	}
	EscapeLevers[0]->SetPosition(XMFLOAT3(87.99f, -0.4077f, 42.959f)); // Broad
	reinterpret_cast<EscapeObject*>(EscapeLevers[0])->SetRotation(DEGREE0);
	EscapeLevers[1]->SetPosition(XMFLOAT3(-43.89f, -0.4077f, 41.97f)); // piano
	reinterpret_cast<EscapeObject*>(EscapeLevers[1])->SetRotation(DEGREE0);
	EscapeLevers[2]->SetPosition(XMFLOAT3(76.65f, -0.4077f, -70.07f)); // forest
	reinterpret_cast<EscapeObject*>(EscapeLevers[2])->SetRotation(DEGREE90);

	if (pLeverModel) delete pLeverModel;
}

void GameScene::BuildObjectsThread(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.0f);
	Material::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.2f);
	LoadedModelInfo* pPlayerModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/C720.bin", nullptr);
	LoadedModelInfo* pClassModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InClassObject.bin", nullptr);
	LoadedModelInfo* pPianoModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InPianoRoom.bin", nullptr);
	LoadedModelInfo* pBroadcastModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InBroadcast.bin", nullptr);
	LoadedModelInfo* pHouseModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InPorest.bin", nullptr);
	LoadedModelInfo* pLobbyModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InDDD.bin", nullptr);
	LoadedModelInfo* pCubeModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CubeRoom.bin", nullptr);
	LoadedModelInfo* pCeilModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Ceilling.bin", nullptr);

	for (int i = 0; i < m_nPlayers; ++i) {
		m_ppPlayers[i]->SetChild(pPlayerModel->m_pModelRootObject, true);
		m_ppPlayers[i]->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 2, pPlayerModel);
		m_ppPlayers[i]->SetAnimation(IDLE);
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 0);
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackEnable(1, false);//m_ppPlayers[i]->SetPosition(XMFLOAT3(i , 0.0f, -5.0f));
		m_ppPlayers[i]->SetAnimationCallback(i);
		for (int j = 0; j < 6; ++j)
			GameObject::SetParts(i + 1, j, 0);
		m_ppPlayers[i]->PlayerNum = i + 1;
		m_ppPlayers[i]->SetPlayerType(TYPE_PLAYER_YET);
	}
	m_ppPlayers[0]->SetPosition(XMFLOAT3(6.0f, 0.0f, -5.0f));
	m_ppPlayers[1]->SetPosition(XMFLOAT3(3.0f, 0.0f, -5.0f));
	m_ppPlayers[2]->SetPosition(XMFLOAT3(-3.0f, 0.0f, -5.0f));
	m_ppPlayers[3]->SetPosition(XMFLOAT3(-6.0f, 0.0f, -5.0f));
	m_ppPlayers[4]->SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));
	
	m_pPlayer->SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pPlayer->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 2, pPlayerModel);
	m_pPlayer->SetAnimation(IDLE);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 0);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pPlayer->SetAnimationCallback(-1);

	m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, -3.0f));
	for (int j = 0; j < 6; ++j)
		GameObject::SetParts(0, j, 0);
	GameObject::SetParts(0, 0, 4);
	m_pPlayer->PlayerNum = 0;
	
	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.3f);
	m_pSkybox = new SkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.35f);
	XMFLOAT3 xmf3Scale(1.0f, 1.0f, 1.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 0.0f);
	m_pMainTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 121, 81, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");
	m_pPianoTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -30, 60, 61, 41, xmf3Scale, xmf4Color, L"Terrain/Floor2.dds");
	m_pBroadcastTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 50, 60, 101, 41, xmf3Scale, xmf4Color, L"Terrain/Floor2.dds");
	m_pCubeTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 80, 0, 41, 81, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");
	m_pForestTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 60, -60, 81, 41, xmf3Scale, xmf4Color, L"Terrain/Road_grass.dds");
	m_pClassroomTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -20, -60, 81, 41, xmf3Scale, xmf4Color, L"Terrain/FloorTex.dds");

	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.5f);
	//UI생성 영역 dds파일 다음 x,y,width,height가 순서대로 들어간다. 아무것도 넣지않으면 화면중앙에 1x1사이즈로 나온다.
	m_nLogin = 6;
	m_UILogin = new GameObject * [m_nLogin];
	m_UILogin[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Login.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UILogin[1] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/LoginButton.dds", -0.67f, -0.55f, 0.2f, 0.14f);
	m_UILogin[2] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/CreateID.dds", -0.38f, -0.55f, 0.2f, 0.14f);
	m_UILogin[3] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Loginfail.dds", 0.0f, -0.2f, 0.8f, 0.35f);
	m_UILogin[4] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/SameID.dds", 0.0f, -0.2f, 0.8f, 0.35f);
	m_UILogin[5] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/SuccessfullycreatedID.dds", 0.0f, -0.2f, 0.8f, 0.35f);

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

	m_nWaitingRoom = 4;
	m_UIWaitingRoom = new GameObject * [m_nWaitingRoom];
	m_UIWaitingRoom[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/WaitingRoom.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UIWaitingRoom[1] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Ready.dds", 0.4f, -0.8f, 0.15f, 0.1f);
	m_UIWaitingRoom[2] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Quit.dds", 0.6f, -0.8f, 0.15f, 0.1f);
	m_UIWaitingRoom[3] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Customizing.dds", 0.8f, -0.8f, 0.15f, 0.1f);

	m_nCustomizing = 11;
	m_UICustomizing = new GameObject * [m_nCustomizing];
	m_UICustomizing[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/CustomizingRoom.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UICustomizing[1] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Ready.dds", 0.6f, -0.8f, 0.15f, 0.1f);
	m_UICustomizing[2] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Quit.dds", 0.8f, -0.8f, 0.15f, 0.1f);
	m_UICustomizing[3] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/HEAD.dds", -0.8f, 0.6f, 0.1f, 0.15f);
	m_UICustomizing[4] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Eyes.dds", -0.8f, 0.4f, 0.1f, 0.15f);
	m_UICustomizing[5] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Mouthandnoses.dds", -0.8f, 0.2f, 0.1f, 0.15f);
	m_UICustomizing[6] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Body.dds", -0.8f, 0.f, 0.1f, 0.15f);
	m_UICustomizing[7] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/BodyParts.dds", -0.8f, -0.2f, 0.1f, 0.15f);
	m_UICustomizing[8] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Gloves.dds", -0.8f, -0.4f, 0.1f, 0.15f);
	m_UICustomizing[9] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/CustomizingRArrow.dds", 0.5f, 0.f, 0.1f, 0.15f);
	m_UICustomizing[10] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/CustomizingLArrow.dds", -0.5f, 0.f, 0.1f, 0.15f);

	m_Ending = 3;
	m_UIEnding = new GameObject * [m_Ending];
	m_UIEnding[0] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Quit.dds", 0.8f, -0.8f, 0.15f, 0.1f);
	m_UIEnding[1] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/SurvivorWin.dds", 0.0f, 0.0f, 2.0f, 2.0f);
	m_UIEnding[2] = new UIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/TaggerWin.dds", 0.0f, 0.0f, 2.0f, 2.0f);

	m_nPlay = 6;
	m_UIPlay = new GameObject * [m_nPlay];
	m_UIPlay[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", 0.8f, -0.75f, 0.3f, 0.4f);
	//m_UIPlay[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", 0.0f, 0.75f, 0.6f, 0.4f); 시계
	m_UIPlay[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/life2.dds", 0.8f, -0.75f, 0.3f, 0.4f);
	m_UIPlay[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Mic-on.dds", 0.5f, -0.85f, 0.07f, 0.15f);//mic-on
	m_UIPlay[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Mic-off.dds", 0.5f, -0.85f, 0.07f, 0.15f);//mic-off
	m_UIPlay[4] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/ChatBox.dds", -0.1f, -0.7f, 0.6f, 0.6f);//chatBox
	m_UIPlay[5] = new MinimapUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Minimap2.dds", 0.0f, 0.0f, 1.0f, 1.0f);//Minimap


	m_nPlayPlayer = 1 + 5 + 1;
	m_UIPlayer = new GameObject * [m_nPlayPlayer];
	m_UIPlayer[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Hammer.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Drill.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Wrench.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[4] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Pliers.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[5] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Driver.dds", -0.75f, -0.75f, 0.3f, 0.4f);
	m_UIPlayer[6] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/tab.dds", -0.65f, -0.6f, 0.1f, 0.1f);

	m_nPlayTagger = 3 + 3;
	m_UITagger = new GameObject * [m_nPlayTagger];
	m_UITagger[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", -0.8f, -0.8f, 0.3f, 0.3f);
	m_UITagger[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", -0.8f, -0.45f, 0.3f, 0.3f);
	m_UITagger[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/Frame.dds", -0.8f, -0.1f, 0.3f, 0.3f);
	m_UITagger[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"SkillImage/ElectronicSystem_Close.dds", -0.8f, -0.8f, 0.3f, 0.3f);
	m_UITagger[4] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"SkillImage/door_emp.dds", -0.8f, -0.45f, 0.3f, 0.3f);
	m_UITagger[5] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"SkillImage/vent_emp.dds", -0.8f, -0.1f, 0.3f, 0.3f);

	m_nAnswerUI = 11;
	m_ppAnswerUIs = new GameObject * [m_nAnswerUI];
	m_ppAnswerUIs[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/solu.dds", -0.4f, 0.0f, 0.6f, 1.2f);
	m_ppAnswerUIs[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.36f, 0.08f, 0.04f);
	m_ppAnswerUIs[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.304f, 0.08f, 0.04f);
	m_ppAnswerUIs[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.248f, 0.08f, 0.04f);
	m_ppAnswerUIs[4] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.192f, 0.08f, 0.04f);
	m_ppAnswerUIs[5] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.136f, 0.08f, 0.04f);
	m_ppAnswerUIs[6] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.08f, 0.08f, 0.04f);
	m_ppAnswerUIs[7] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, 0.024f, 0.08f, 0.04f);
	m_ppAnswerUIs[8] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, -0.032f, 0.08f, 0.04f);
	m_ppAnswerUIs[9] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, -0.088f, 0.08f, 0.04f);
	m_ppAnswerUIs[10] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/node.dds", -0.46f, -0.144f, 0.08f, 0.04f);

	m_nInfoUI = 5;
	m_ppInfoUIs = new GameObject * [m_nInfoUI];
	m_ppInfoUIs[0] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/FixSuccess.dds", 0.0f, 0.7f, 1.0f, 0.3f);
	m_ppInfoUIs[1] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/FixFail.dds", 0.0f, 0.7f, 1.0f, 0.3f);
	m_ppInfoUIs[2] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/ActivateEscapeSystem.dds", 0.0f, 0.7f, 1.0f, 0.3f);
	m_ppInfoUIs[3] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/WorkingEscapeSystem.dds", 0.0f, 0.7f, 1.0f, 0.3f);
	m_ppInfoUIs[4] = new IngameUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/EscapeSuccess.dds", 0.0f, 0.7f, 1.0f, 0.3f);

	LPVOID m_pTerrain[ROOM_COUNT]{ m_pMainTerrain ,m_pPianoTerrain,m_pBroadcastTerrain, m_pCubeTerrain ,m_pForestTerrain,m_pClassroomTerrain };

	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.7f);
	m_pCeilling = new GameObject();
	m_pCeilling->SetChild(pCeilModel->m_pModelRootObject, true);
	m_pCeilling->UpdateTransform(nullptr);
	LoadSceneObjectsFromFile(pd3dDevice, pd3dCommandList, (char*)"Walls/Scene0621.bin");

	m_pPVSObjects[0] = new GameObject();
	m_pPVSObjects[0]->SetChild(pClassModel->m_pModelRootObject, true);
	m_pPVSObjects[0]->UpdateTransform(nullptr);
	m_pPVSObjects[1] = new GameObject();
	m_pPVSObjects[1]->SetChild(pPianoModel->m_pModelRootObject, true);
	m_pPVSObjects[1]->UpdateTransform(nullptr);
	m_pPVSObjects[2] = new GameObject();
	m_pPVSObjects[2]->SetChild(pBroadcastModel->m_pModelRootObject, true);
	m_pPVSObjects[2]->UpdateTransform(nullptr);
	m_pPVSObjects[3] = new GameObject();
	m_pPVSObjects[3]->SetChild(pLobbyModel->m_pModelRootObject, true);
	m_pPVSObjects[3]->UpdateTransform(nullptr);
	m_pPVSObjects[4] = new GameObject();
	m_pPVSObjects[4]->SetChild(pHouseModel->m_pModelRootObject, true);
	m_pPVSObjects[4]->UpdateTransform(nullptr);
	m_pPVSObjects[5] = new GameObject();
	m_pPVSObjects[5]->SetChild(pCubeModel->m_pModelRootObject, true);
	m_pPVSObjects[5]->UpdateTransform(nullptr);
	/*	m_pClass = new GameObject();
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
	m_pPorest->UpdateTransform(nullptr);*/

	LoadSceneBushFromFile(pd3dDevice, pd3dCommandList, (char*)"Model/Bush.bin");

	m_pPlayer->SetPlayerUpdatedContext(m_pTerrain);
	m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
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
	if (pCubeModel) delete pCubeModel;

	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.8f);
	m_nObjectsUIs = 9;
	m_ppObjectsUIs = new InteractionUI * [m_nObjectsUIs];
	m_ppObjectsUIs[0] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fOpen.dds");
	m_ppObjectsUIs[1] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fClose.dds");
	m_ppObjectsUIs[2] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fPick.dds");
	m_ppObjectsUIs[3] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fRepair.dds");
	m_ppObjectsUIs[4] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/cCancel.dds");
	m_ppObjectsUIs[5] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/block.dds");
	m_ppObjectsUIs[6] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fCollect.dds");
	m_ppObjectsUIs[7] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fEscape.dds");
	m_ppObjectsUIs[8] = new InteractionUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Texture/fActivate.dds");

	MakeVents(pd3dDevice, pd3dCommandList);
	MakeDoors(pd3dDevice, pd3dCommandList);
	MakePowers(pd3dDevice, pd3dCommandList);
	MakeBoxes(pd3dDevice, pd3dCommandList);
	MakeTaggers(pd3dDevice, pd3dCommandList);
	MakeEscapeLevers(pd3dDevice, pd3dCommandList);
	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(0.9f);
#if USE_NETWORK
	m_network = Network::GetInstance();
	m_network->init_network();
	m_network->m_pPlayer = m_pPlayer;
	m_network->m_ppOther = m_ppPlayers;
	m_network->m_Taggers_Box = Taggers;
	m_network->m_UIPlay = m_UIPlay;

	for (int i = 0; i < 6; ++i)
		m_network->m_pDoors[i] = m_pDoors[i];

	for (int i = 0; i < 5; ++i)
		m_network->m_pPowers[i] = m_pPowers[i];

	for (int i = 0; i < NUM_ITEMBOX; ++i)
		m_network->m_pBoxes[i] = m_pBoxes[i];

	for (int i = 0; i < NUM_VENT; ++i)
		m_network->m_Vents[i] = Vents[i];

	for (int i = 0; i < NUM_ESCAPE_LEVER; ++i)
		m_network->m_EscapeLevers[i] = EscapeLevers[i];

	recv_thread = std::thread{ &Network::listen_thread, m_network };

	m_pPlayer->SetPlayerType(TYPE_PLAYER_YET);
	for (int i = 0; i < 5; ++i)
		m_ppPlayers[i]->SetPlayerType(TYPE_PLAYER_YET);
#endif
#if USE_CHAT_TEST
	send_thread = std::thread{ &Network::Debug_send_thread, m_network };
#endif
	reinterpret_cast<IngameUI*>(m_UILoading[2])->SetGuage(1.0f);

	Input::GetInstance()->m_pTestDoor = m_pDoors[0];
}