#pragma once
class GameObject;

class Component
{
public:
	GameObject* gameObject;
public:
	virtual void start(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void update(float elapsedTime) = 0;
};