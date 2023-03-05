#pragma once
#include "stdafx.h"

enum Object_Type{OB_PIANO, OB_DOOR, OB_DESK };

class GameObject {
private:
	XMFLOAT3				m_pos;
	float					m_width;
	float					m_height;

	Object_Type				m_type;

	BoundingOrientedBox		m_bounding_box;
public:
	GameObject();
	~GameObject();

	void					init(Object_Type, float width, float height);
	void					Set_BoundingBox();
	BoundingOrientedBox		Get_BoundingBox();
};