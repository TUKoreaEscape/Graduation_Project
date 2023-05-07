#pragma once
#include "Input.h"
#include "Player.h"
#include "Scene.h"
#include "GameScene.h"
#include "Time.h"
#include "Renderer.h"
#include "Shader.h"
#include "GameObject.h"

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
	
	ID3D12CommandQueue *m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pd3dCommandList; //��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�.

	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];//

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;//
	//D3D12_CPU_DESCRIPTOR_HANDLE* m_d3dDepthStencilBufferSRVCPUHandle;//
	
	ID3D12Fence *m_pd3dFence;
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	HANDLE m_hFenceEvent; //�潺 �������̽� ������, �潺�� ��, �̺�Ʈ �ڵ��̴�.
	
	Input*			input;
	GameScene*		scene;
	Time			time;

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
	void CreateCommandQueueAndList(); //���� ü��, ����̽�, ������ ��, ��� ť/�Ҵ���/����Ʈ�� �����ϴ� �Լ��̴�.

	void CreateRenderTargetViews();
	void CreateDepthStencilView(); //���� Ÿ�� ��� ����-���ٽ� �並 �����ϴ� �Լ��̴�. 
	void CreateSwapChainRenderTargetViews();

	void BuildObjects();
	void ReleaseObjects(); //�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�.

	void UpdateObjects(); //AnimateObjects
	void FrameAdvance();
	//�����ӿ�ũ�� �ٽ�(�ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�. 

	void WaitForGpuComplete(); //CPU�� GPU�� ����ȭ�ϴ� �Լ��̴�.

	void ChangeSwapChainState();

	void MoveToNextFrame();
};

