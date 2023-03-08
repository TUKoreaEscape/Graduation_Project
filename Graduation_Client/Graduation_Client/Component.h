#pragma once
class Component
{
public:
	class GameObject* gameObject;
public:
	virtual void start() = 0;
	virtual void update(float elapsedTime) = 0;
};

class MaterialComponent : Component
{
public:
	virtual void start() = 0;
	virtual void update(float elapsedTime) = 0;
};

class MeshComponent : Component
{
public:
	virtual void start() = 0;
	virtual void update(float elapsedTime) = 0;
};

class ShaderComponent : Component
{
public:
};