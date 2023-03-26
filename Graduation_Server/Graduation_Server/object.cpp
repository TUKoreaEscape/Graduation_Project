#include "object.h"
#include "GameServer.h"

GameObject::GameObject()
{

}

GameObject::GameObject(Object_Type type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	m_type = type;
	m_center = center;
	m_extents = extents;
	m_orientation = orientation;
	BoundingOrientedBox bounding_box{ center, extents,orientation };
	Set_BoundingBox(bounding_box);
}

void GameObject::Set_BoundingBox(const BoundingOrientedBox& box)
{
	m_bounding_box = box;
}

BoundingOrientedBox GameObject::Get_BoundingBox()
{
	return m_bounding_box;
}

Object_Type GameObject::Get_Object_Type()
{
	return m_type;
}

XMFLOAT3 GameObject::Get_center()
{
	return m_center;
}

XMFLOAT3 GameObject::Get_extents()
{
	return m_extents;
}

XMFLOAT4 GameObject::Get_orientation()
{
	cGameServer& server = cGameServer::GetInstance();
	return m_orientation;
}

Door::Door()
{
	m_pos = { 0.0f, 0.0f, 0.0f };
	m_center = { 0.0f, 0.0f, 0.0f };
	m_extents = { 0.0f, 0.0f, 0.0f };
	m_orientation = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_type = Object_Type::OB_DOOR;
}
