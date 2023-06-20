#pragma once
#include "Input.h"
#include "Player.h"
#include "Scene.h"
#include "GameScene.h"
#include "Time.h"
#include "Renderer.h"
#include "Shader.h"
#include "GameObject.h"
#include "Game_state.h"
#include "UI.h"

class Framework
{
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_nWndClientWidth;
	int m_nWndClientHeight;

	IDXGIFactory4* m_pdxgiFactory; // 팩토리 인터페이스 포인터
	IDXGISwapChain3* m_pdxgiSwapChain; //스왑 체인 인터페이스
	ID3D12Device* m_pd3dDevice; // 디바이스 인터페이스 포인터

	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0; //MSAA 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다.

	static const UINT m_nSwapChainBuffers = 2; //스왑 체인의 후면 버퍼의 개수이다.
	UINT m_nSwapChainBufferIndex; //현재 스왑 체인의 후면 버퍼 인덱스이다.

	ID3D12Resource *m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	//ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];

	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	
	ID3D12Resource *m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	ID3D12DescriptorHeap* m_pd3dDsvShadowDescriptorHeap;

	ID3D12Resource* m_pd3dShadowMapBuffer;
	
	ID3D12CommandQueue *m_pd3dCommandQueue;
	//ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12CommandAllocator* m_ppd3dCommandAllocators[m_nSwapChainBuffers];
	ID3D12GraphicsCommandList* m_pd3dCommandList; //명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다.

	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];//

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;//
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvShadowDescriptorCPUHandle;//

	//D3D12_CPU_DESCRIPTOR_HANDLE* m_d3dDepthStencilBufferSRVCPUHandle;//
	
	ID3D12Fence *m_pd3dFence;
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	HANDLE m_hFenceEvent; //펜스 인터페이스 포인터, 펜스의 값, 이벤트 핸들이다.

	//D2D코드
	ID3D11On12Device* m_pd3d11On12Device = NULL;
	ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
	ID2D1Factory3* m_pd2dFactory = NULL;
	IDWriteFactory* m_pdWriteFactory = NULL;
	ID2D1Device2* m_pd2dDevice = NULL;
	ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;

	ID3D11Resource* m_ppd3d11WrappedBackBuffers[m_nSwapChainBuffers];
	ID2D1Bitmap1* m_ppd2dRenderTargets[m_nSwapChainBuffers];

	ID2D1SolidColorBrush* m_pd2dbrBackground = NULL;
	ID2D1SolidColorBrush* m_pd2dbrBorder = NULL;
	IDWriteTextFormat* m_pdLoginFont = NULL;
	IDWriteTextFormat* m_pdRoomTitleFont = NULL;
	IDWriteTextFormat* m_pdRoomOtherFont = NULL;
	IDWriteTextLayout* m_pdwTextLayout = NULL;
	IDWriteTextLayout* m_pdRoomTitleFLayout = NULL;
	IDWriteTextLayout* m_pdRoomOtherLayout = NULL;

	ID2D1SolidColorBrush* m_pd2dpurpleText = NULL;
	ID2D1SolidColorBrush* m_pd2dlightsalmonText = NULL;
	ID2D1SolidColorBrush* m_pd2dblackText = NULL;
	
	Input*			input;
	GameScene*		scene;
	Time			time;
	GameState*	m_gamestate;

	float           timeToSend;

	LaplacianEdgeShader		*m_pEdgeShader = NULL;

	_TCHAR						m_pszFrameRate[70];

	int								m_nDebugOptions = 10; // U
public:
	Framework();
	~Framework();
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd); //프레임워크를 초기화하는 함수이다(주 윈도우가 생성되면 호출된다). 
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDirect3DDevice();
	void CreateDirect2DDevice();
	void CreateCommandQueueAndList(); //스왑 체인, 디바이스, 서술자 힙, 명령 큐/할당자/리스트를 생성하는 함수이다.

	void CreateRenderTargetViews();
	void CreateDepthStencilView(); //렌더 타겟 뷰와 깊이-스텐실 뷰를 생성하는 함수이다. 
	void CreateShadowDepthStencilView(); //쉐도우 맵 깊이-스텐실 뷰를 생성했다.
	void CreateSwapChainRenderTargetViews();

	void BuildObjects();
	void ReleaseObjects(); //렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수이다.

	void UpdateObjects(); //AnimateObjects
	void FrameAdvance();
	//프레임워크의 핵심(애니메이션, 렌더링)을 구성하는 함수이다. 

	void WaitForGpuComplete(); //CPU와 GPU를 동기화하는 함수이다.

	void ChangeSwapChainState();

	void MoveToNextFrame();

	void TextRender();
};

/*
* szRenderTarget.width / 9.14, szRenderTarget.height / 1.62, szRenderTarget.width / 2.73, szRenderTarget.height / 1.51 -> ID칸 //175,553,585,595
* szRenderTarget.width / 9.14, szRenderTarget.height / 1.46, szRenderTarget.width / 2.73, szRenderTarget.height / 1.37 / 4칸 175, 615, 585, 657
* 
*/
