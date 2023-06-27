#pragma once
#define DEBUG 1
#define PRINT 0
#define SOCKET_ERROR_PRINT 0
#define MAX_USER 30000
#define NUMBER_OF_DOOR 6
#define NUMBER_OF_ELECTRONIC 5

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <mutex>
#include <array>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <queue>
#include <concurrent_priority_queue.h>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <locale.h>
#include <random>
#include <ctime>
#include <chrono>
#include "EXPOver.h"

// Windows 헤더 파일:
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <fstream>
#include <vector>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
//#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <D3d12SDKLayers.h>
#include <Mmsystem.h>

#include <concurrent_priority_queue.h>


using namespace std;
using namespace DirectX;

struct CollisionInfo
{
	bool	 is_collision;
	int	     collision_face_num;
	XMFLOAT3 CollisionNormal; // 충돌 면의 법선 벡터
	XMFLOAT3 SlidingVector;
};

inline float DistanceToPlane(XMFLOAT3 point, XMFLOAT3 normal, XMFLOAT3 planePoint)
{
	XMVECTOR pointVec = XMLoadFloat3(&point);
	XMVECTOR normalVec = XMLoadFloat3(&normal);
	XMVECTOR planePointVec = XMLoadFloat3(&planePoint);
	return XMVectorGetX(XMPlaneDotCoord(XMPlaneFromPointNormal(planePointVec, normalVec), pointVec));

}

inline float Dot(XMFLOAT3 a, XMFLOAT3 b)
{
	XMVECTOR aVec = XMLoadFloat3(&a);
	XMVECTOR bVec = XMLoadFloat3(&b);
	return XMVectorGetX(XMVector3Dot(aVec, bVec));
}

inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}

inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
	return(xmf3Result);
}

inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}

inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector)
{
	XMFLOAT4 xmf4Result;
	XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
	return(xmf4Result);
}

namespace CLIENT_STATE
{
	enum STATE {
		ST_FREE, ST_ACCEPT, ST_LOBBY, ST_GAMEROOM, ST_INGAME
	};
}

namespace CLIENT_ROLE
{
	enum STATE {
		ROLE_NONE, ROLE_RUNNER, ROLE_TAGGER
	};
}

