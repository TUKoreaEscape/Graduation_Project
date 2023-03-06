#pragma once
#include "stdafx.h"

enum Object_Type{NONE, OB_PIANO, OB_DOOR, OB_DESK };

class GameObject {
private:
	Object_Type				m_type = NONE;
	XMFLOAT3				m_center;
	XMFLOAT3				m_extents;
	XMFLOAT4				m_orientation;

	BoundingOrientedBox		m_bounding_box;
public:
	GameObject();
	GameObject(Object_Type type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	~GameObject();

	void					Set_BoundingBox();
	BoundingOrientedBox		Get_BoundingBox();

	Object_Type				Get_Object_Type();
	XMFLOAT3				Get_center();
	XMFLOAT3				Get_extents();
	XMFLOAT4				Get_orientation();

	friend istream& operator>>(istream& in, GameObject& p);
};