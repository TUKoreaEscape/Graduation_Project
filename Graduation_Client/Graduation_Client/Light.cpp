#include "stdafx.h"
#include "Light.h"
#include "Game_state.h"

void Light::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nLights = 3;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	float global = 0.5f;
	m_xmf4GlobalAmbient = XMFLOAT4(global, global, global, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

	float lightpower = 1.0f;
	m_pLights[1].m_bEnable = false;
	m_pLights[1].m_nType = POINT_LIGHT;
	m_pLights[1].m_fRange = 50.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(lightpower, lightpower, lightpower, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(lightpower, lightpower, lightpower, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0, 5.f, 0.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, .0f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	
	m_pLights[2].m_bEnable = false;
	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 30.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 3.0f, 20.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[2].m_fFalloff = 2.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void Light::update(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights
}

void Light::Updaterotate()
{
	if (GameState::GetInstance()->GetInitLight())
	{
		rotationAngle = 270.0f;
		rotationSpeed = 0.25f;

		float global = 0.5f;
		m_xmf4GlobalAmbient = XMFLOAT4(global, global, global, 1.0f);

		m_pLights[0].m_bEnable = true;
		m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
		m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		m_pLights[0].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
		m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		GameState::GetInstance()->SetInitLight();
	}
	// ȸ�� ���� ������Ʈ
	rotationAngle += rotationSpeed;

	// z �� ȸ���� ���� �ﰢ�Լ� ���
	float radians = XMConvertToRadians(rotationAngle);
	float sinAngle = sinf(radians);
	float cosAngle = cosf(radians);

	// ���ο� ���� ��ġ �� ���� ���
	m_pLights[0].m_xmf3Direction.x = cosAngle;
	m_pLights[0].m_xmf3Direction.y = sinAngle;

	if (m_pLights[0].m_xmf3Direction.y < 0)
	{
		float global = m_pLights[0].m_xmf3Direction.y * -0.5 + 0.2f;
		m_xmf4GlobalAmbient = XMFLOAT4(global, global, global, 1.0f);

		float diffuse = m_pLights[0].m_xmf3Direction.y * -1.0f;
		if (diffuse > 0.3f) diffuse = 0.3;
		m_pLights[0].m_xmf4Diffuse = XMFLOAT4(diffuse, diffuse, diffuse, 1.0f);

		float ambient = m_pLights[0].m_xmf3Direction.y * -1.0;
		m_pLights[0].m_xmf4Ambient = XMFLOAT4(ambient, ambient, ambient, 1.0f);
	}
	else
	{
		float ambient = 0.1;
		m_pLights[0].m_xmf4Ambient = XMFLOAT4(ambient, ambient, ambient, 1.0f);
	}
	if (m_pLights[0].m_xmf3Direction.x == 1)// ���̵Ǵ� ���� x���� ���� �۾��� // x�� 1�϶� ��9�� y�� -1�϶�
	{
		float global = 0.2f;
		m_xmf4GlobalAmbient = XMFLOAT4(global, global, global, 1.0f);
		m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		m_pLights[0].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (m_pLights[0].m_xmf3Direction.x == -1)// ���̵Ǵ� ���� x���� ���� Ŀ��
	{
		m_pLights[0].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	}
	//x�� 1���Ĵ� ��, y�� -1���Ĵ� ��
	//std::cout << m_pLights[0].m_xmf3Direction.x << "   " << m_pLights[0].m_xmf3Direction.y << std::endl;
	//m_pLights[0].m_xmf3Direction = XMFLOAT3(-lightPosition.x, -lightPosition.y, -lightPosition.z);
}

void Light::SetWaitingLight(bool set)
{
	m_pLights[2].m_bEnable = set;
	m_pLights[0].m_bEnable = !set;
}
