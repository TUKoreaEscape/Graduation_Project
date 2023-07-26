#pragma once
#include "stdafx.h"
#include "Framework.h"

const float SEND_TIME = 0.02f;

Framework::Framework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	m_bMsaa4xEnable = false;
	m_nMsaa4xQualityLevels = 0;

	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_ppd3dRenderTargetBuffers[i] = NULL;
		m_ppd3dCommandAllocators[i] = NULL;
		m_nFenceValues[i] = 0;
		m_ppd3d11WrappedBackBuffers[i] = NULL;
		m_ppd2dRenderTargets[i] = NULL;
	}
	m_pd3dRtvDescriptorHeap = NULL;
	
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDsvDescriptorHeap = NULL;
	
	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	scene = NULL;

	timeToSend = 0.0f;


	_tcscpy_s(m_pszFrameRate, _T("Who's the Tagger? ("));
}

Framework::~Framework()
{

}

bool Framework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateSwapChainRenderTargetViews();
	CreateDepthStencilView(); //Direct3D 디바이스, 명령 큐와 명령 리스트, 스왑 체인 등을 생성하는 함수를 호출한다.

	CreateDirect2DDevice();

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	BuildObjects(); //렌더링할 게임 객체를 생성한다.

#if USE_NETWORK
	Network& network = *Network::GetInstance();
	network.m_hwnd = hMainWnd;
#endif

	return(true);
}

void Framework::OnDestroy()
{
	WaitForGpuComplete(); //GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
#if USE_NETWORK
	Network& network = *Network::GetInstance();
	network.send_thread.join();
	network.~Network();
	scene->recv_thread.join();
#endif
	Sound& sound = *Sound::GetInstance();
	sound.~Sound();

	ReleaseObjects(); //게임 객체(게임 월드 객체)를 소멸한다.
	::CloseHandle(m_hFenceEvent);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

	if (m_pd2dbrBackground) m_pd2dbrBackground->Release();
	if (m_pd2dbrBorder) m_pd2dbrBorder->Release();
	if (m_pdLoginFont) m_pdLoginFont->Release();
	if (m_pdRoomTitleFont) m_pdRoomTitleFont->Release();
	if (m_pdRoomOtherFont) m_pdRoomOtherFont->Release();
	if (m_pdwTextLayout) m_pdwTextLayout->Release();
	if (m_pd2dlightsalmonText) m_pd2dlightsalmonText->Release();
	if (m_pd2dpurpleText) m_pd2dpurpleText->Release();
	if (m_pd2dblackText) m_pd2dblackText->Release();

	if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
	if (m_pd2dDevice) m_pd2dDevice->Release();
	if (m_pdWriteFactory) m_pdWriteFactory->Release();
	if (m_pd3d11On12Device) m_pd3d11On12Device->Release();
	if (m_pd3d11DeviceContext) m_pd3d11DeviceContext->Release();
	if (m_pd2dFactory) m_pd2dFactory->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3d11WrappedBackBuffers[i]) m_ppd3d11WrappedBackBuffers[i]->Release();
		if (m_ppd2dRenderTargets[i]) m_ppd2dRenderTargets[i]->Release();
		if (m_ppd3dCommandAllocators[i]) m_ppd3dCommandAllocators[i]->Release();
	}

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

void Framework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE; //전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다.
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	//CreateRenderTargetViews();
#endif
}

void Framework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + 5;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
}

void Framework::CreateDirect3DDevice()
{
	HRESULT hResult;
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void
		**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i,&pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,_uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	} //모든 하드웨어 어댑터 대하여 특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성한다.

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}//특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스를 생성한다.
	if (pd3dAdapter) pd3dAdapter->Release();

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels; //디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false; //다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다.
	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 1;
	//m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	/*펜스와 동기화를 위한 이벤트 객체를 생성한다(이벤트 객체의 초기값을 FALSE이다).
	이벤트가 실행되면(Signal) 이벤트의 값을 자동적으로 FALSE가 되도록 생성한다.*/

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	
}

void Framework::CreateDirect2DDevice()
{
	UINT nD3D11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG) || defined(DBG)
	nD3D11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device* pd3d11Device = NULL;
	ID3D12CommandQueue* ppd3dCommandQueues[] = { m_pd3dCommandQueue };
	HRESULT hResult = ::D3D11On12CreateDevice(m_pd3dDevice, nD3D11DeviceFlags, NULL, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, &pd3d11Device, &m_pd3d11DeviceContext, NULL);
	hResult = pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&m_pd3d11On12Device);
	if (pd3d11Device) pd3d11Device->Release();

	D2D1_FACTORY_OPTIONS nD2DFactoryOptions = { D2D1_DEBUG_LEVEL_NONE };
#if defined(_DEBUG) || defined(DBG)
	nD2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	ID3D12InfoQueue* pd3dInfoQueue = NULL;
	if (SUCCEEDED(m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
	{
		D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO,
		};

		D3D12_MESSAGE_ID pd3dDenyIds[] =
		{
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
		};

		D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
		d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
		d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
		d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
		d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

		pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
	}
	pd3dInfoQueue->Release();
#endif

	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &nD2DFactoryOptions, (void**)&m_pd2dFactory);

	IDXGIDevice* pdxgiDevice = NULL;
	hResult = m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);
	hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, &m_pd2dDevice);
	hResult = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
	hResult = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pdWriteFactory);
	if (pdxgiDevice) pdxgiDevice->Release();

	m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.0f, 0.0f, 0.5f), &m_pd2dbrBackground);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0x9ACD32, 1.0f)), &m_pd2dbrBorder);

	hResult = m_pdWriteFactory->CreateTextFormat(L"나눔스퀘어 ExtraBold", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-US", &m_pdLoginFont);
	hResult = m_pdWriteFactory->CreateTextFormat(L"굴림체", NULL, DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"en-US", &m_pdRoomTitleFont);
	hResult = m_pdWriteFactory->CreateTextFormat(L"굴림체", NULL, DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 50.0f, L"en-US", &m_pdReadytoStartFont);
	hResult = m_pdWriteFactory->CreateTextFormat(L"굴림체", NULL, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-US", &m_pdRoomOtherFont);
	hResult = m_pdWriteFactory->CreateTextFormat(L"굴림체", NULL, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"ko-KR", &m_pdChatFont);
	hResult = m_pdLoginFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdLoginFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	hResult = m_pdRoomTitleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	hResult = m_pdRoomTitleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple, 1.0f), &m_pd2dpurpleText);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSalmon, 1.0f), &m_pd2dlightsalmonText);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dblackText);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CadetBlue, 1.0f), &m_pd2dCadetBlueText);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 8, m_pdLoginFont, 4096.0f, 4096.0f, &m_pdwTextLayout);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 8, m_pdRoomTitleFont, 4096.0f, 4096.0f, &m_pdRoomTitleFLayout);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 8, m_pdReadytoStartFont, 4096.0f, 4096.0f, &m_pdRoomTitleFLayout);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 8, m_pdRoomOtherFont, 4096.0f, 4096.0f, &m_pdRoomOtherLayout);

	float fDpi = (float)GetDpiForWindow(m_hWnd);
	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fDpi, fDpi);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		m_pd3d11On12Device->CreateWrappedResource(m_ppd3dRenderTargetBuffers[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_ppd3d11WrappedBackBuffers[i]));
		IDXGISurface* pdxgiSurface = NULL;
		m_ppd3d11WrappedBackBuffers[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
		m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
		if (pdxgiSurface) pdxgiSurface->Release();
	}
}

void Framework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);//직접(Direct) 명령 큐를 생성한다.

	for (UINT i = 0; i < m_nSwapChainBuffers; i++) hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_ppd3dCommandAllocators[i]); //직접(Direct) 명령 할당자를 생성한다.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ppd3dCommandAllocators[0], NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList); //직접(Direct) 명령 리스트를 생성한다.

	hResult = m_pd3dCommandList->Close(); //명령 리스트는 생성되면 열린(Open) 상태이므로 닫힌(Closed) 상태로 만든다.

	// buildThread용 커맨드리스트
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_ppd3dBuildCommandAllocators); //직접(Direct) 명령 할당자를 생성한다.
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ppd3dBuildCommandAllocators, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dBuildCommandList); //직접(Direct) 명령 리스트를 생성한다.
	hResult = m_pd3dBuildCommandList->Close();
}

void Framework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void Framework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT; // 후처리 기술을 사용할때 문제가 생긴다면 이부분도 봐야함 DXGI_FORMAT_D32_FLOAT
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	//D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer); //깊이-스텐실 버퍼를 생성한다.

	m_d3dDsvDescriptorCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, m_d3dDsvDescriptorCPUHandle); //깊이-스텐실 버퍼 뷰를 생성한다.
}

void Framework::CreateSwapChainRenderTargetViews()
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dSwapChainBackBufferRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void Framework::BuildObjects()
{
	//m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	m_pd3dCommandList->Reset(m_ppd3dCommandAllocators[m_nSwapChainBufferIndex], NULL);
	input = Input::GetInstance();
	m_gamestate = GameState::GetInstance();
	network = Network::GetInstance();
	sound = Sound::GetInstance();
	sound->StartFMOD();
	m_gamestate->SetBG();
	input->m_gamestate = m_gamestate;
	scene = new GameScene();
	scene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	m_pEdgeShader = new LaplacianEdgeShader();
	m_pEdgeShader->CreateShader(m_pd3dDevice, scene->GetGraphicsRootSignature(), 1, NULL, DXGI_FORMAT_D32_FLOAT);
	//m_pEdgeShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);

	DXGI_FORMAT pdxgiResourceFormats[6] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32_FLOAT };
	m_pEdgeShader->CreateResourcesAndViews(m_pd3dDevice, 6, pdxgiResourceFormats, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, d3dRtvCPUDescriptorHandle, 7); //SRV to (Render Targets) + (Depth Buffer)

	DXGI_FORMAT pdxgiDepthSrvFormats[1] = { DXGI_FORMAT_R32_FLOAT };
	m_pEdgeShader->CreateShaderResourceViews(m_pd3dDevice, 1, &m_pd3dDepthStencilBuffer, pdxgiDepthSrvFormats);

	LIGHT* lights = (LIGHT*)reinterpret_cast<Light*>(scene->m_pLight->GetComponent<Light>())->m_pLights;
	m_pDepthRenderShader = new DepthRenderShader(lights,scene);
	DXGI_FORMAT pdxgiRtvFormats[1] = { DXGI_FORMAT_R32_FLOAT };
	m_pDepthRenderShader->CreateShader(m_pd3dDevice, scene->GetGraphicsRootSignature(), 1, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	m_pDepthRenderShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, NULL);

	DXGI_FORMAT RtvFormats[6] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32_FLOAT };
	m_pShadowMapShader = new ShadowMapShader(scene);
	m_pShadowMapShader->CreateShader(m_pd3dDevice, scene->GetGraphicsRootSignature(), 6, RtvFormats, DXGI_FORMAT_D32_FLOAT);
	m_pShadowMapShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pDepthRenderShader->GetDepthTexture());

	m_pShadowMapToViewport = new TextureToViewportShader();
	m_pShadowMapToViewport->CreateShader(m_pd3dDevice, scene->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_pShadowMapToViewport->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pDepthRenderShader->GetDepthTexture());

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	buildThread = std::thread{ &Framework::BuildObjectsThread, this };

	//scene->ReleaseUploadBuffers();

	time.Reset();
}

void Framework::ReleaseObjects()
{
	if (scene) scene->ReleaseObjects();
	if (scene) delete scene;
}

void Framework::UpdateObjects()
{
	float fTimeElapsed = time.GetTimeElapsed();
	timeToSend += fTimeElapsed;

#if USE_NETWORK
	if (input ->m_pPlayer->GetID() != -1 && (m_gamestate->GetGameState() == READY_TO_GAME || m_gamestate->GetGameState() == PLAYING_GAME || m_gamestate->GetGameState() == SPECTATOR_GAME) ) {
		cs_packet_move packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_MOVE;
		packet.velocity = input->m_pPlayer->GetVelocity();
		packet.xmf3Shift = input->m_pPlayer->GetShift();
		packet.input_key = input->m_pPlayer->GetDirection();

		packet.look.x = input->m_pPlayer->GetLookVector().x * 100;
		packet.look.y = input->m_pPlayer->GetLookVector().y * 100;
		packet.look.z = input->m_pPlayer->GetLookVector().z * 100;

		packet.right.x = input->m_pPlayer->GetRightVector().x * 100;
		packet.right.y = input->m_pPlayer->GetRightVector().y * 100;
		packet.right.z = input->m_pPlayer->GetRightVector().z * 100;

		//packet.yaw = input->m_pPlayer->GetYaw();
		packet.is_jump = input->m_pPlayer->GetIsFalling();
		//std::cout << packet.xmf3Shift.x << ", " << packet.xmf3Shift.y << ", " << packet.xmf3Shift.z << std::endl;
		//std::cout << packet.look.x << ", " << packet.look.y << ", " << packet.look.z << std::endl;
		//std::cout << packet.right.x << ", " << packet.right.y << ", " << packet.right.z << std::endl;
		if (Input::GetInstance()->m_pPlayer->GetType() != TYPE_ESCAPE_PLAYER) {
			network->send_packet(&packet);

			//while (!network.m_recv_move);
			//network->pos_lock.lock();
			input->m_pPlayer->SetPosition(network->m_pPlayer_Pos);
		}
		//network->pos_lock.unlock();

		for (int i = 0; i < 5; ++i)
		{
			if (network->m_ppOther[i]->GetID() != -1)
			{
				//network->Other_Player_Pos[i].pos_lock.lock();
				network->m_ppOther[i]->SetPosition(network->Other_Player_Pos[i].Other_Pos);
				//network->Other_Player_Pos[i].pos_lock.unlock();
			}
		}
	}
#endif

	scene->update(fTimeElapsed, m_pd3dDevice, m_pd3dCommandList);
	if (scene->m_pPlayer->m_pCamera->m_pcbMappedCamera == nullptr)
	{
		scene->m_pPlayer->m_pCamera->start(m_pd3dDevice, m_pd3dCommandList);
	}
}

#define _WITH_PLAYER_TOP

void Framework::FrameAdvance()
{
	time.Tick(60.0);

	HRESULT hResult;
	D3D12_VIEWPORT d3dViewport = { 0.0f, 0.0f, FRAME_BUFFER_WIDTH * 0.25f, FRAME_BUFFER_HEIGHT * 0.25f, 0.0f, 1.0f };
	D3D12_RECT d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH / 4, FRAME_BUFFER_HEIGHT / 4 };

	if (m_gamestate->GetGameState() == GAME_LOADING) {
		hResult = WaitForSingleObject(buildThread.native_handle(), 30);
		if (hResult == WAIT_TIMEOUT) {
			hResult = m_ppd3dCommandAllocators[m_nSwapChainBufferIndex]->Reset();
			hResult = m_pd3dCommandList->Reset(m_ppd3dCommandAllocators[m_nSwapChainBufferIndex], NULL); //명령 할당자와 명령 리스트를 리셋한다.

			::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
			m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
			scene->Loadingrender(m_pd3dCommandList);

			::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			hResult = m_pd3dCommandList->Close();
			ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

			m_pdxgiSwapChain->Present(0, 0);
			MoveToNextFrame();

			time.GetFrameRate(m_pszFrameRate + 19, 37);
			::SetWindowText(m_hWnd, m_pszFrameRate);
			return;
		}
		else if (hResult == WAIT_OBJECT_0) {
			m_gamestate->ChangeNextState();
			buildThread.join();
		}
	}
	hResult = m_ppd3dCommandAllocators[m_nSwapChainBufferIndex]->Reset();
	hResult = m_pd3dCommandList->Reset(m_ppd3dCommandAllocators[m_nSwapChainBufferIndex], NULL); //명령 할당자와 명령 리스트를 리셋한다.

	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	input->Update(m_hWnd);

	UpdateObjects();
	if (input->keyBuffer['1'] & 0xF0) m_nDebugOptions = 85;
	else m_nDebugOptions = 10;

	switch (m_gamestate->GetGameState()) {
	case LOGIN:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		scene->UIrender(m_pd3dCommandList);
		break;
	case ROOM_SELECT:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		scene->UIrender(m_pd3dCommandList);
		break;
	case WAITING_GAME:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		scene->UIrender(m_pd3dCommandList);
		scene->WaitingRoomrender(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions); 
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		break;
	case CUSTOMIZING:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		scene->UIrender(m_pd3dCommandList);
		scene->WaitingRoomrender(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		break;
	case READY_TO_GAME:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pDepthRenderShader->PrepareShadowMap(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		if (scene) scene->defrender(m_pd3dCommandList);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions);
		m_pEdgeShader->Render(m_pd3dCommandList);
		scene->UIrender(m_pd3dCommandList); // Door UI
		break;
	case PLAYING_GAME:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pDepthRenderShader->PrepareShadowMap(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);

		m_pDepthRenderShader->UpdateShaderVariables(m_pd3dCommandList);
		m_pShadowMapShader->UpdateShaderVariables(m_pd3dCommandList);
		if (scene) scene->defrender(m_pd3dCommandList);

		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		//m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions);
		//m_pEdgeShader->Render(m_pd3dCommandList);
		scene->UIrender(m_pd3dCommandList); // Door UI
		break;
	case ENDING_GAME:
		if (scene) scene->prerender(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		scene->UIrender(m_pd3dCommandList); 
		scene->Endingrender(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions); 
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		break;
	case SPECTATOR_GAME:
		if (scene) scene->prerender(m_pd3dCommandList); 
		m_pDepthRenderShader->PrepareShadowMap(m_pd3dCommandList);
		if (scene) scene->SpectatorPrerender(m_pd3dCommandList);
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		if (scene) scene->Spectatorrender(m_pd3dCommandList);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions);
		m_pEdgeShader->Render(m_pd3dCommandList);
		break;
	case INTERACTION_POWER:
		if (scene) scene->prerender(m_pd3dCommandList); 
		m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pEdgeShader->OnPrepareRenderTarget(m_pd3dCommandList, 1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], m_d3dDsvDescriptorCPUHandle);
		if (scene) scene->defrender(m_pd3dCommandList);
		m_pEdgeShader->OnPostRenderTarget(m_pd3dCommandList);
		m_pEdgeShader->UpdateShaderVariables(m_pd3dCommandList, &m_nDebugOptions);
		m_pEdgeShader->Render(m_pd3dCommandList);
		scene->UIrender(m_pd3dCommandList); // Door UI
		break;
	}
	//m_pd3dCommandList->OMSetRenderTargets(1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDescriptorCPUHandle);
	//if(Input::GetInstance()->keyBuffer['1'] & 0xF0) m_pEdgeShader->Render(m_pd3dCommandList);


	//::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	/*현재 렌더 타겟에 대한 렌더링이 끝나기를 기다린다. GPU가 렌더 타겟(버퍼)을 더 이상 사용하지 않으면 렌더 타겟
	의 상태는 프리젠트 상태(D3D12_RESOURCE_STATE_PRESENT)로 바뀔 것이다.*/
	
	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dCommandList->Close(); //명령 리스트를 닫힌 상태로 만든다.
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists); //명령 리스트를 명령 큐에 추가하여 실행한다.
	WaitForGpuComplete(); //GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	
	TextRender();//text 렌더

	m_pdxgiSwapChain->Present(0, 0);
	/*스왑체인을 프리젠트한다. 프리젠트를 하면 현재 렌더 타겟(후면버퍼)의 내용이 전면버퍼로 옮겨지고 렌더 타겟 인덱스가 바뀔 것이다.*/
	MoveToNextFrame();

	time.GetFrameRate(m_pszFrameRate + 19, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

}

void Framework::WaitForGpuComplete()
{
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue); //GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다.
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) //펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다.
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void Framework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews();
}

void Framework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void Framework::TextRender()
{
	if (m_gamestate->GetGameState() == LOGIN)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		int size = strlen(input->m_cs_packet_login.id);
		if (size > 0)
		{
			wchar_t* array = new wchar_t[size + 1];
			for (int i = 0; i < size; i++) {
				array[i] = static_cast<wchar_t>(input->m_cs_packet_login.id[i]);
			}
			array[size] = '\0';
			D2D1_RECT_F rcLowerText = D2D1::RectF(idRect.left, idRect.top, idRect.right, idRect.bottom);
			m_pd2dDeviceContext->DrawTextW(array, (UINT32)wcslen(array), m_pdLoginFont, &rcLowerText, m_pd2dpurpleText);
			delete[] array;
		}

		size = strlen(input->m_cs_packet_login.pass_word);
		if (size > 0)
		{
			wchar_t* array = new wchar_t[size + 1];
			for (int i = 0; i < size; i++) {
				array[i] = static_cast<wchar_t>(input->m_cs_packet_login.pass_word[i]);
			}
			array[size] = '\0';
			D2D1_RECT_F rcLowerText = D2D1::RectF(passwordRect.left, passwordRect.top, passwordRect.right, passwordRect.bottom);
			m_pd2dDeviceContext->DrawTextW(array, (UINT32)wcslen(array), m_pdLoginFont, &rcLowerText, m_pd2dlightsalmonText);
			delete[] array;
		}

		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == ROOM_SELECT)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		for (int i = 0; i < 6; ++i)
		{
			int size = strlen(input->m_Roominfo[i].room_name);
			wchar_t* array = new wchar_t[size + 1];
			for (int j = 0; j < size; j++) {
				array[j] = static_cast<wchar_t>(input->m_Roominfo[i].room_name[j]);
			}
			array[size] = '\0';
			D2D1_RECT_F rcLowerText = D2D1::RectF(roominfoRect[i].left+10, roominfoRect[i].top + 30, roominfoRect[i].right+10, roominfoRect[i].bottom + 30);
			m_pd2dDeviceContext->DrawTextW(array, (UINT32)wcslen(array), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);

			rcLowerText = D2D1::RectF(roominfoRect[i].left, roominfoRect[i].top + 0, roominfoRect[i].right, roominfoRect[i].bottom + 0);
			m_pd2dDeviceContext->DrawTextW(L"방 제목", (UINT32)wcslen(L"방 제목"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);

			std::wstring num = std::to_wstring(input->m_Roominfo[i].room_number);
			rcLowerText = D2D1::RectF(roominfoRect[i].left+10, roominfoRect[i].top+30, roominfoRect[i].right+10, roominfoRect[i].bottom+30);
			m_pd2dDeviceContext->DrawTextW(num.c_str(), (UINT32)wcslen(num.c_str()), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			rcLowerText = D2D1::RectF(roominfoRect[i].left + 10, roominfoRect[i].top + 10, roominfoRect[i].right + 10, roominfoRect[i].bottom + 10);
			m_pd2dDeviceContext->DrawTextW(L"방 번호", (UINT32)wcslen(L"방 번호"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			std::wstring joinnum = std::to_wstring(input->m_Roominfo[i].join_member);
			rcLowerText = D2D1::RectF(roominfoRect[i].left + 300, roominfoRect[i].top+30, roominfoRect[i].right + 300, roominfoRect[i].bottom+30);
			m_pd2dDeviceContext->DrawTextW(joinnum.c_str(), (UINT32)wcslen(joinnum.c_str()), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			rcLowerText = D2D1::RectF(roominfoRect[i].left + 300, roominfoRect[i].top + 10, roominfoRect[i].right + 300, roominfoRect[i].bottom + 10);
			m_pd2dDeviceContext->DrawTextW(L"방 인원", (UINT32)wcslen(L"방 인원"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			rcLowerText = D2D1::RectF(roominfoRect[i].left + 350, roominfoRect[i].top + 30, roominfoRect[i].right + 350, roominfoRect[i].bottom + 30);
			m_pd2dDeviceContext->DrawTextW(L"6", (UINT32)wcslen(L"6"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			rcLowerText = D2D1::RectF(roominfoRect[i].left + 340, roominfoRect[i].top + 80, roominfoRect[i].right + 340, roominfoRect[i].bottom + 80);
			m_pd2dDeviceContext->DrawTextW(L"방 상태", (UINT32)wcslen(L"방 상태"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);

			switch (input->m_Roominfo[i].state) {
			case GAME_ROOM_STATE::FREE:
				rcLowerText = D2D1::RectF(roominfoRect[i].left + 350, roominfoRect[i].top + 100, roominfoRect[i].right + 350, roominfoRect[i].bottom + 100);
				m_pd2dDeviceContext->DrawTextW(L"FREE", (UINT32)wcslen(L"FREE"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);
				break;
			case GAME_ROOM_STATE::READY:
				rcLowerText = D2D1::RectF(roominfoRect[i].left +  350, roominfoRect[i].top + 100, roominfoRect[i].right + 350, roominfoRect[i].bottom + 100);
				m_pd2dDeviceContext->DrawTextW(L"READY", (UINT32)wcslen(L"READY"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);
				break;
			case GAME_ROOM_STATE::PLAYING:
				rcLowerText = D2D1::RectF(roominfoRect[i].left + 350, roominfoRect[i].top + 100, roominfoRect[i].right + 350, roominfoRect[i].bottom + 100);
				m_pd2dDeviceContext->DrawTextW(L"PLAYING", (UINT32)wcslen(L"PLAYING"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);
				break;
			case GAME_ROOM_STATE::END:
				rcLowerText = D2D1::RectF(roominfoRect[i].left + 350, roominfoRect[i].top + 100, roominfoRect[i].right + 350, roominfoRect[i].bottom + 100);
				m_pd2dDeviceContext->DrawTextW(L"END", (UINT32)wcslen(L"END"), m_pdRoomOtherFont, &rcLowerText, m_pd2dblackText);
				break;
			}
			delete[] array;
		}

		std::wstring num = std::to_wstring(input->m_PageNum);
		D2D1_RECT_F rcLowerText = D2D1::RectF(pageNumRect.left, pageNumRect.top, pageNumRect.right, pageNumRect.bottom);
		m_pd2dDeviceContext->DrawTextW(num.c_str(), (UINT32)wcslen(num.c_str()), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);

		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == WAITING_GAME)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		for (int i = 0; i < 3; ++i)
		{
			D2D1_RECT_F rcLowerText = D2D1::RectF(waitingRoomRect[i].left, waitingRoomRect[i].top, waitingRoomRect[i].right, waitingRoomRect[i].bottom);
			if (i == 0 && !input->m_cs_packet_ready.ready_type) m_pd2dDeviceContext->DrawTextW(L"READY", (UINT32)wcslen(L"READY"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
			else if (i == 0 && input->m_cs_packet_ready.ready_type) m_pd2dDeviceContext->DrawTextW(L"READY OK", (UINT32)wcslen(L"READY OK"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
			else if (i == 1) m_pd2dDeviceContext->DrawTextW(L"QUIT", (UINT32)wcslen(L"QUIT"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
			else if (i == 2) m_pd2dDeviceContext->DrawTextW(L"CUSTOMIZING", (UINT32)wcslen(L"CUSTOMIZING"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
		}

		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == CUSTOMIZING)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		for (int i = 0; i < 2; ++i)
		{
			D2D1_RECT_F rcLowerText = D2D1::RectF(customizingRect[i].left, customizingRect[i].top, customizingRect[i].right, customizingRect[i].bottom);
			if (i == 0 ) m_pd2dDeviceContext->DrawTextW(L"SAVE", (UINT32)wcslen(L"SAVE"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
			else if (i == 1) m_pd2dDeviceContext->DrawTextW(L"QUIT", (UINT32)wcslen(L"QUIT"), m_pdRoomTitleFont, &rcLowerText, m_pd2dblackText);
		}

		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == READY_TO_GAME)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		if (m_gamestate->IsLoading())
		{
			D2D1_RECT_F rcLowerText = D2D1::RectF(m_nWndClientWidth / 10, m_nWndClientHeight / 2 + 200, m_nWndClientWidth / 3, m_nWndClientHeight + 200);
			switch(m_gamestate->LoadingCount()){
			case 0:
				m_pd2dDeviceContext->DrawTextW(L"Loading", (UINT32)wcslen(L"Loading"), m_pdReadytoStartFont, &rcLowerText, m_pd2dCadetBlueText);
				break;
			case 1:
				m_pd2dDeviceContext->DrawTextW(L"Loading.", (UINT32)wcslen(L"Loading."), m_pdReadytoStartFont, &rcLowerText, m_pd2dCadetBlueText);
				break;
			case 2:
				m_pd2dDeviceContext->DrawTextW(L"Loading..", (UINT32)wcslen(L"Loading.."), m_pdReadytoStartFont, &rcLowerText, m_pd2dCadetBlueText);
				break;
			case 3:
				m_pd2dDeviceContext->DrawTextW(L"Loading...", (UINT32)wcslen(L"Loading..."), m_pdReadytoStartFont, &rcLowerText, m_pd2dCadetBlueText);
				break;
			}
		}
		else
		{
			D2D1_RECT_F rcLowerText = D2D1::RectF(m_nWndClientWidth / 2 - 250, m_nWndClientHeight / 2 - 300, m_nWndClientWidth / 2 + 500, m_nWndClientHeight + 200);
			m_pd2dDeviceContext->DrawTextW(L"곧 술래가 정해집니다.", (UINT32)wcslen(L"곧 술래가 정해집니다."), m_pdReadytoStartFont, &rcLowerText, m_pd2dblackText);

			std::wstring num = std::to_wstring(m_gamestate->GetTaggerTime());
			rcLowerText = D2D1::RectF(m_nWndClientWidth / 2 - 25, m_nWndClientHeight / 2 - 250, m_nWndClientWidth / 2 + 500, m_nWndClientHeight + 250);
			m_pd2dDeviceContext->DrawTextW(num.c_str(), (UINT32)wcslen(num.c_str()), m_pdReadytoStartFont, &rcLowerText, m_pd2dlightsalmonText);
		}

		m_pd2dDeviceContext->EndDraw();

		m_gamestate->UpdateLoading(time.GetTimeElapsed());

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == PLAYING_GAME)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		if(m_gamestate->GetChatState())
		{
			//내가 지금 입력하고있는 문자열 출력해야함
			int size = strlen(input->m_cs_packet_chat.message);
			if (size > 0)
			{
				wchar_t* array = new wchar_t[size + 1];
				//for (int j = 0; j < size; j++) {
				//	array[j] = static_cast<wchar_t>(input->m_cs_packet_chat.message[j]);
				//}
				setlocale(LC_CTYPE, "ko-KR");
				mbstowcs(array, input->m_cs_packet_chat.message, size);
				array[size] = '\0';
				D2D1_RECT_F rcLowerText = D2D1::RectF(chatBoxRect.left, chatBoxRect.top, chatBoxRect.right, chatBoxRect.bottom);
				m_pd2dDeviceContext->DrawTextW(array, (UINT32)wcslen(array), m_pdChatFont, &rcLowerText, m_pd2dpurpleText);
				delete[] array;
				//setlocale(LC_CTYPE, "en-US");
			}
			for (int i = 0; i < 5; ++i)
			{
				int size = strlen(input->m_chatlist[i]);
				if (size > 0)
				{
					wchar_t* array = new wchar_t[size + 1];
					//for (int j = 0; j < size; j++) {
					//	array[j] = static_cast<wchar_t>(input->m_chatlist[i][j]);
					//}
					setlocale(LC_CTYPE, "ko-KR");
					mbstowcs(array, input->m_chatlist[i], size);
					array[size] = '\0';
					D2D1_RECT_F rcLowerText = D2D1::RectF(chatBoxRect.left, chatBoxRect.top - i * 40 - 55, chatBoxRect.right, chatBoxRect.bottom - i * 40 - 55);
					m_pd2dDeviceContext->DrawTextW(array, (UINT32)wcslen(array), m_pdChatFont, &rcLowerText, m_pd2dpurpleText);
					delete[] array;
					//setlocale(LC_CTYPE, "en-US");
				}
			}
		}

		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
	else if (m_gamestate->GetGameState() == ENDING_GAME)
	{
		m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
		ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
		m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd2dDeviceContext->BeginDraw();

		m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

		D2D1_RECT_F rcLowerText = D2D1::RectF(endingRect.left, endingRect.top, endingRect.right, endingRect.bottom);
		m_pd2dDeviceContext->DrawTextW(L"QUIT", (UINT32)wcslen(L"QUIT"), m_pdRoomOtherLayout, &rcLowerText, m_pd2dblackText);
		m_pd2dDeviceContext->EndDraw();

		m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

		m_pd3d11DeviceContext->Flush();
	}
}

void Framework::BuildObjectsThread()
{
	m_pd3dBuildCommandList->Reset(m_ppd3dBuildCommandAllocators, NULL);
	scene->BuildObjectsThread(m_pd3dDevice, m_pd3dBuildCommandList);
	m_pd3dBuildCommandList->Close();
	ID3D12CommandList* ppd3dBuildCommandLists[] = { m_pd3dBuildCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dBuildCommandLists);

	WaitForGpuComplete();

	scene->ReleaseUploadBuffers();
}
