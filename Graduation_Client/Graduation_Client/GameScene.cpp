#include "stdafx.h"
#include "Light.h"
#include "GameScene.h"
#include "Movement.h"
#include "Network.h"

E_GAME_STATE gameState;
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
	gameState = E_GAME_RUNNING;
}

void GameScene::render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	m_pPlayer->m_pCamera->update(pd3dCommandList);
	m_pLight->GetComponent<Light>()->update(pd3dCommandList);

	m_pSkybox->render(pd3dCommandList);

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
}

void GameScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 66);

	Material::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	LoadedModelInfo* pPlayerModel = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/P02.bin", nullptr); 
	//LoadedModelInfo* pPlayerModel2 = GameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/P02.bin", nullptr);

	m_pNPC = new GameObject();
	m_pNPC->SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pNPC->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
	m_pNPC->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_pNPC->m_pSkinnedAnimationController->SetTrackPosition(0, 0.5f);
	m_pNPC->m_pSkinnedAnimationController->SetTrackSpeed(0, 2.0f);
	m_pNPC->m_xmf4x4ToParent._42 = 100.0f;

	m_nPlayers = 5;
	m_ppPlayers = new Player * [m_nPlayers];
	for (int i = 0; i < m_nPlayers; ++i) {
		m_ppPlayers[i] = new Player();
		m_ppPlayers[i]->SetChild(pPlayerModel->m_pModelRootObject, true);
		m_ppPlayers[i]->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
		m_ppPlayers[i]->SetPosition(XMFLOAT3(0, 1.f, i * 3.f));
		m_ppPlayers[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	}
	m_ppPlayers[0]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
	m_ppPlayers[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppPlayers[1]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
	m_ppPlayers[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppPlayers[2]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
	m_ppPlayers[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppPlayers[3]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
	m_ppPlayers[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppPlayers[4]->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.f);
	m_ppPlayers[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	
	m_pPlayer = new Player();
	m_pPlayer->SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pPlayer->m_pSkinnedAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, 1, pPlayerModel);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(0, 1.0f);
	m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));

	LoadSceneObjectsFromFile(pd3dDevice, pd3dCommandList, (char*)"Walls/Scene.bin");
	
	if (pPlayerModel) delete pPlayerModel;
	
	m_pSkybox = new SkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_pLight = new GameObject();
	m_pLight->AddComponent<Light>();
	m_pLight->start(pd3dDevice, pd3dCommandList);
	
	XMFLOAT3 xmf3Scale(1.0f, 1.0f, 1.0f);
	XMFLOAT4 xmf4Color(1.f, 1.f, 1.f, 0.0f);
	m_pMainTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 121, 81, xmf3Scale, xmf4Color);
	xmf4Color = XMFLOAT4(1.f, 1.f, 0.f, 0.0f);
	m_pPianoTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -30, 60, 61, 41, xmf3Scale, xmf4Color);
	xmf4Color = XMFLOAT4(1.f, 0.f, 0.f, 0.0f);
	m_pBroadcastTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 50, 60, 101, 41, xmf3Scale, xmf4Color);
	xmf4Color = XMFLOAT4(0.f, 1.f, 0.f, 0.0f);
	m_pCubeTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 80, 0, 41, 81, xmf3Scale, xmf4Color);
	xmf4Color = XMFLOAT4(0.f, 0.f, 0.f, 0.0f);
	m_pForestTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 60, -60, 81, 41, xmf3Scale, xmf4Color);
	xmf4Color = XMFLOAT4(0.f, 1.f, 1.f, 0.0f);
	m_pClassroomTerrain = new HeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), -20, -60, 81, 41, xmf3Scale, xmf4Color);

	m_pPlayer->AddComponent<CommonMovement>(); 

	AddPlayer(m_pNPC);
	for (int i = 0; i < m_nPlayers; ++i) {
		AddPlayer(m_ppPlayers[i]);
	}
	AddPlayer(m_pPlayer);


#if USE_NETWORK
	m_network = Network::GetInstance();
	m_network->init_network();
	m_network->m_pPlayer = m_pPlayer;
	m_network->m_ppOther = m_ppPlayers;
	recv_thread = std::thread{ &Network::listen_thread, m_network };
	cs_packet_login l_packet;
	l_packet.size = sizeof(l_packet);
	l_packet.type = CS_PACKET::CS_LOGIN;
	strcpy_s(l_packet.id, sizeof("test1"), "test1");
	strcpy_s(l_packet.pass_word, sizeof("test1"), "test1");
	m_network->send_packet(&l_packet);
#endif
}

void GameScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
}

ID3D12RootSignature* GameScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

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


	D3D12_ROOT_PARAMETER pd3dRootParameters[15];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
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
}

void GameScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

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

	WallShader* pShader = new WallShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
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

		Material* pMaterial = new Material(7);
		pMaterial->m_ppTextures[0] = new Texture(1, RESOURCE_TEXTURE2D, 0, 1);
		pMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Walls/stones.dds", RESOURCE_TEXTURE2D, 0);

		CreateShaderResourceViews(pd3dDevice, pMaterial->m_ppTextures[0], 0, 3);

		pMaterial->SetShader(pShader);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
		pGameObject->renderer->SetMaterial(pMaterial);

		m_ppWalls[i] = pGameObject;
	}

	::fclose(pFile);
}
