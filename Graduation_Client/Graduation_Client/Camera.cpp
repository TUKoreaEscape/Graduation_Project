#include "stdafx.h"
#include "Player.h"
#include "Camera.h"
#include "Input.h"

//Camera::Camera()
//{
//	m_xmf4x4View = Matrix4x4::Identity();
//	m_xmf4x4Projection = Matrix4x4::Identity();
//	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
//	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
//	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
//	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
//	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
//	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
//	m_fPitch = 0.0f;
//	m_fRoll = 0.0f;
//	m_fYaw = 0.0f;
//	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
//	m_fTimeLag = 0.0f;
//	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
//	m_nMode = 0x00;
//	m_pPlayer = NULL;
//}
//
//Camera::Camera(Camera* pCamera)
//{
//	if (pCamera)
//	{
//		*this = *pCamera;
//	}
//	else
//	{
//		m_xmf4x4View = Matrix4x4::Identity();
//		m_xmf4x4Projection = Matrix4x4::Identity();
//		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
//		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
//		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
//		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
//		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
//		m_fPitch = 0.0f;
//		m_fRoll = 0.0f;
//		m_fYaw = 0.0f;
//		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_fTimeLag = 0.0f;
//		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_nMode = 0x00;
//		m_pPlayer = NULL;
//	}
//}
//
//Camera::~Camera()
//{
//}

void Camera::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	//m_pPlayer = reinterpret_cast<Player*>(this->targetObject);

	//gameObject->m_xmf4x4ToParent._41;
	//m_xmf3Position = XMFLOAT3(gameObject->m_xmf4x4ToParent._41, gameObject->m_xmf4x4ToParent._42, gameObject->m_xmf4x4ToParent._43);
	//m_xmf3Position = Vector3::Add(m_xmf3Position, m_xmf3Offset);
	//m_xmf3Position = XMFLOAT3(0.0f, 2.5f, -5.0f);
	//m_xmf4x4View = Matrix4x4::LookAtLH(XMFLOAT3(0.0f, 2.5f, -5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	//m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	
	//m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(60.0f),
	//	float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT), 1.01f, 500.0f);
	//m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	//m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Camera::update(float elapsedTime)
{
	RegenerateViewMatrix();
}

void Camera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void Camera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void Camera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	//	XMMATRIX xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	//	XMStoreFloat4x4(&m_xmf4x4Projection, xmmtxProjection);
}

void Camera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void Camera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void Camera::RegenerateViewMatrix()
{
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
}

void Camera::update(ID3D12GraphicsCommandList* pd3dCommandList)
{
	SetViewportsAndScissorRects(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList);
}

void Camera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256�� ���
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);
}

void Camera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4InverseProjection, XMMatrixTranspose(XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4Projection))));
	::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));
	::memcpy(&m_pcbMappedCamera->m_xmf3Direction, &m_xmf3Look, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void Camera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
	}
}

void Camera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// SpaceShipCamera
//
//void SpaceShipCamera::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
//{
//	Camera::start(pd3dDevice, pd3dCommandList);
//	m_nMode = SPACESHIP_CAMERA;
//	m_pPlayer = static_cast<Player*>(this->targetObject);
//}
//
//void SpaceShipCamera::Rotate(float x, float y, float z)
//{
//	if (m_pPlayer && (x != 0.0f))
//	{
//		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Right), XMConvertToRadians(x));
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
//		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
//		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
//	}
//	if (m_pPlayer && (y != 0.0f))
//	{
//		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
//		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
//		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
//	}
//	if (m_pPlayer && (z != 0.0f))
//	{
//		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
//		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
//		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
//	}
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FirstPersonCamera

void FirstPersonCamera::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Camera::start(pd3dDevice, pd3dCommandList);
	m_nMode = FIRST_PERSON_CAMERA;
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Right.y = 0.0f;
	m_xmf3Look.y = 0.0f;
	m_xmf3Right = Vector3::Normalize(m_xmf3Right);
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_pPlayer = Input::GetInstance()->m_pPlayer;
	SetOffset(XMFLOAT3(0.0f, 1.5f, 0.0f));
	GenerateProjectionMatrix(0.2f, 5000.0f, ASPECT_RATIO, 60.0f);
	SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	SetPosition(Vector3::Add(m_xmf3Position, m_xmf3Offset));
}

void FirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ThirdPersonCamera

void ThirdPersonCamera::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Camera::start(pd3dDevice, pd3dCommandList);
	m_nMode = THIRD_PERSON_CAMERA;
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Right.y = 0.0f;
	m_xmf3Look.y = 0.0f;
	m_xmf3Right = Vector3::Normalize(m_xmf3Right);
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_pPlayer = Input::GetInstance()->m_pPlayer;
	SetTimeLag(0.05f);
	SetOffset(XMFLOAT3(0.0f, 3.0f, 5.0f));
	GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 75.0f);
	SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	//m_pPlayer->SetPosition(XMFLOAT3(0, 0, 0));
	if (m_pPlayer)
	{
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);
		float fDistance = fLength;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(m_pPlayer->GetPosition());
		}
	}
}

void ThirdPersonCamera::update(float elapsedTime)
{
	Camera::update(elapsedTime);
}

void ThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer->GetUpVector());
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ThirdPersonCamera

void CustomizingCamera::start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Camera::start(pd3dDevice, pd3dCommandList);
	m_nMode = THIRD_PERSON_CAMERA;
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Right.y = 0.0f;
	m_xmf3Look.y = 0.0f;
	m_xmf3Right = Vector3::Normalize(m_xmf3Right);
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_pPlayer = Input::GetInstance()->m_pPlayer;
	SetTimeLag(0.05f);
	SetOffset(XMFLOAT3(0.0f, 3.0f, 5.0f));
	GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 75.0f);
	SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	//m_pPlayer->SetPosition(XMFLOAT3(0, 0, 0));
	if (m_pPlayer)
	{
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);
		float fDistance = fLength;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(m_pPlayer->GetPosition());
		}
	}
}

void CustomizingCamera::update(float elapsedTime)
{
	Camera::update(elapsedTime);
	//if (m_pPlayer)
	//{
	//	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
	//	XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
	//	XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
	//	XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
	//	xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
	//	xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
	//	xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

	//	XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
	//	XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
	//	XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
	//	float fLength = Vector3::Length(xmf3Direction);
	//	xmf3Direction = Vector3::Normalize(xmf3Direction);
	//	float fTimeLagScale = (m_fTimeLag) ? elapsedTime * (1.0f / m_fTimeLag) : 1.0f;
	//	float fDistance = fLength * fTimeLagScale;
	//	if (fDistance > fLength) fDistance = fLength;
	//	if (fLength < 0.01f) fDistance = fLength;
	//	if (fDistance > 0)
	//	{
	//		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
	//		SetLookAt(m_pPlayer->GetPosition());
	//	}
	//}
}

void CustomizingCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer->GetUpVector());
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}