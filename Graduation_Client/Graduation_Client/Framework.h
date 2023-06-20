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

	IDXGIFactory4* m_pdxgiFactory; // ���丮 �������̽� ������
	IDXGISwapChain3* m_pdxgiSwapChain; //���� ü�� �������̽�
	ID3D12Device* m_pd3dDevice; // ����̽� �������̽� ������

	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0; //MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�.

	static const UINT m_nSwapChainBuffers = 2; //���� ü���� �ĸ� ������ �����̴�.
	UINT m_nSwapChainBufferIndex; //���� ���� ü���� �ĸ� ���� �ε����̴�.

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
	ID3D12GraphicsCommandList* m_pd3dCommandList; //��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�.

	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];//

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;//
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvShadowDescriptorCPUHandle;//

	//D3D12_CPU_DESCRIPTOR_HANDLE* m_d3dDepthStencilBufferSRVCPUHandle;//
	
	ID3D12Fence *m_pd3dFence;
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	HANDLE m_hFenceEvent; //�潺 �������̽� ������, �潺�� ��, �̺�Ʈ �ڵ��̴�.

	//D2D�ڵ�
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
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd); //�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�(�� �����찡 �����Ǹ� ȣ��ȴ�). 
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDirect3DDevice();
	void CreateDirect2DDevice();
	void CreateCommandQueueAndList(); //���� ü��, ����̽�, ������ ��, ��� ť/�Ҵ���/����Ʈ�� �����ϴ� �Լ��̴�.

	void CreateRenderTargetViews();
	void CreateDepthStencilView(); //���� Ÿ�� ��� ����-���ٽ� �並 �����ϴ� �Լ��̴�. 
	void CreateShadowDepthStencilView(); //������ �� ����-���ٽ� �並 �����ߴ�.
	void CreateSwapChainRenderTargetViews();

	void BuildObjects();
	void ReleaseObjects(); //�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�.

	void UpdateObjects(); //AnimateObjects
	void FrameAdvance();
	//�����ӿ�ũ�� �ٽ�(�ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�. 

	void WaitForGpuComplete(); //CPU�� GPU�� ����ȭ�ϴ� �Լ��̴�.

	void ChangeSwapChainState();

	void MoveToNextFrame();

	void TextRender();
};

/*
* szRenderTarget.width / 9.14, szRenderTarget.height / 1.62, szRenderTarget.width / 2.73, szRenderTarget.height / 1.51 -> IDĭ //175,553,585,595
* szRenderTarget.width / 9.14, szRenderTarget.height / 1.46, szRenderTarget.width / 2.73, szRenderTarget.height / 1.37 / 4ĭ 175, 615, 585, 657
* 
*/
