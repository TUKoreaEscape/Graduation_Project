#pragma once
#include "stdafx.h"

enum Object_Type{NONE, OB_PIANO, OB_DOOR, OB_DESK };

class GameObject {
private:
	XMFLOAT3				m_pos;
	float					m_width = 0;
	float					m_height = 0;

	Object_Type				m_type = NONE;

	BoundingOrientedBox		m_bounding_box;
public:
	GameObject();
	GameObject(Object_Type type, float width, float height);
	~GameObject();

	void					init(Object_Type, float width, float height);
	void					Set_BoundingBox();
	BoundingOrientedBox		Get_BoundingBox();
};