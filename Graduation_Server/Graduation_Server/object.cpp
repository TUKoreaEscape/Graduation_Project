#include "object.h"

GameObject::GameObject()
{

}

GameObject::GameObject(Object_Type type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation)
{
	m_type = type;
	m_center = center;
	m_extents = extents;
	m_orientation = orientation;

	Set_BoundingBox();
}

GameObject::~GameObject()
{

}

void GameObject::Set_BoundingBox()
{
	m_bounding_box = BoundingOrientedBox{ m_center, m_extents, m_orientation };
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
	return m_orientation;
}

istream& operator>>(istream& in, GameObject& p)
{
	in.read(reinterpret_cast<char*>(&p), sizeof(GameObject));
	return in;
}