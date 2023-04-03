#pragma once
#include "stdafx.h"

enum Object_Type{NONE, OB_PIANO, OB_DOOR, OB_DESK, OB_WALL };

class GameObject {
protected:
	Object_Type				m_type = NONE;
	XMFLOAT3				m_center = {0, 0, 0};
	XMFLOAT3				m_extents = {0, 0, 0};
	XMFLOAT4				m_orientation = { 0, 0, 0, 0 };

	XMFLOAT3				m_pos;

	BoundingOrientedBox		m_bounding_box;
public:
	GameObject();
	GameObject(Object_Type type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	virtual ~GameObject() = default;

	void					Set_BoundingBox(const BoundingOrientedBox& box);
	void					Set_Position(XMFLOAT3 pos);

	BoundingOrientedBox		Get_BoundingBox();

	Object_Type				Get_Object_Type();
	XMFLOAT3				Get_center();
	XMFLOAT3				Get_extents();
	XMFLOAT4				Get_orientation();

	void					Update_bounding_box_pos(const XMFLOAT3 pos);
	void					Update_bounding_box_rotate(const float yaw);

	virtual void			send_event(const unsigned int id) {};
};

class Door : public GameObject { // 인게임 도어에 관련된 부분 (도어 오픈 시간 차이도 있으므로 체크해줘야함)
public:
	Door();
	virtual ~Door() = default;

	void send_event(const unsigned int id) override {};
};