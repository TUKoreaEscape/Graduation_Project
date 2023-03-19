#pragma once
#include "Component.h"

class CommonMovement : public Component
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void update(float elapsedTime);
};

class TaggerMovement : public Component
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};
	void update(float elapsedTime) {};
};

class SurvivorMovement : public Component
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};
	void update(float elapsedTime) {};
};

class DeadMovement : public Component
{
public:
	void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};
	void update(float elapsedTime) {};
};