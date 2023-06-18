#pragma once
#include "stdafx.h"

enum Object_Type{NONE, OB_PIANO, OB_DOOR, OB_DESK, OB_WALL, OB_ELECTRONICSYSTEM };

class GameObject {
protected:
	Object_Type				m_type = NONE;
	XMFLOAT3				m_center = {0, 0, 0};
	XMFLOAT3				m_extents = {0, 0, 0};
	XMFLOAT4				m_orientation = { 0, 0, 0, 0 };

	XMFLOAT3				m_pos;

	BoundingOrientedBox		m_bounding_box;

	int						m_section = -1;
public:
	GameObject();
	GameObject(Object_Type type, XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 orientation);
	virtual ~GameObject() = default;

	void					Set_BoundingBox(const BoundingOrientedBox& box);
	void					Set_Position(XMFLOAT3 pos) { m_pos = pos; }
	void					Set_ObjectSection(int section) { m_section = section; }
	void					Update_bounding_box_rotate(const float yaw);

	BoundingOrientedBox		Get_BoundingBox();

	Object_Type				Get_Object_Type();
	XMFLOAT3				Get_center();
	XMFLOAT3				Get_extents();
	XMFLOAT4				Get_orientation();

	int						Get_Section() { return m_section; }

	virtual void			Update_bounding_box_pos(const XMFLOAT3& pos);
	virtual void			send_event(const unsigned int id) {}
	virtual void			Update_Object() {};
};

enum Door_State{ST_OPEN, ST_OPENING ,ST_CLOSE};

class Door : public GameObject { // �ΰ��� ��� ���õ� �κ� (���� ���� �ð� ���̵� �����Ƿ� üũ�������)
private:
	int			m_door_id;
	Door_State	m_state = ST_CLOSE;


public:
	mutex*								m_state_lock = nullptr;
	chrono::system_clock::time_point	m_door_open_start_time;
	chrono::system_clock::time_point	m_door_close_start_time;
	float								m_door_open_duration = 400.f;
	bool								m_door_open_start = false;
	bool								m_door_close_start = false;
	bool								m_check_bounding_box = true;
public:
	Door();
	Door(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents);
	~Door() = default;

	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object();

	bool process_door_event();

	void set_boundingbox_check(bool option) { m_check_bounding_box = option; }

	void Release();
	Door_State get_state() { return m_state; }
};

class ElectronicSystem : public GameObject {
private:
	int		m_system_id = -1;
	bool	m_correct_on_off_switch[15];
public:
	ElectronicSystem();
	ElectronicSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents);
	~ElectronicSystem() = default;

	void init_electrinic_switch_data(int idx, bool value) { m_correct_on_off_switch[idx] = value; }

	bool Get_On_Off_Switch_Vaild(int idx, bool data[]);
	bool Get_On_Off_Switch_Value(int idx) { return m_correct_on_off_switch[idx]; }
	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object();
};

class EscapeSystem : public GameObject {
private:
	int		m_system_id;

public:
	EscapeSystem();
	EscapeSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents);
	~EscapeSystem() = default;

	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object();
};