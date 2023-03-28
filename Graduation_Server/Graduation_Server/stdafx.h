#pragma once
#define DEBUG 1
#define PRINT 1
#define MAX_USER 30000
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
#include <mutex>
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