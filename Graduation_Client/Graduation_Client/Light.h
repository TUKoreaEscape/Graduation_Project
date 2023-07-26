#pragma once
#include "stdafx.h"
#include "Component.h"

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class Light: public Component
{
public:
	virtual void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void update(float elapsedTime) {}
	
	virtual void update(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Updaterotate();
	LIGHT* GetLights() const { return m_pLights; }
public:
	LIGHT* m_pLights = nullptr;
	int m_nLights = 0;
	float global = 0.7f;
	// Directional Light의 회전 각도
	float rotationAngle = 270.0f;

	// Directional Light의 회전 속도
	float rotationSpeed = 0.25f;
	bool dayAndnight = true; //  true일때 낮

	XMFLOAT4 m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = nullptr;
	LIGHTS* m_pcbMappedLights = nullptr;

	void SetWaitingLight(bool set);
};