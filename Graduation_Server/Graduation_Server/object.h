#pragma once
#include "stdafx.h"

enum Object_Type{NONE, OB_DOOR, OB_FIX, OB_WALL, OB_ELECTRONICSYSTEM };

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
enum ES_State{ES_OPEN, ES_CLOSE};

class Door : public GameObject { // 인게임 도어에 관련된 부분 (도어 오픈 시간 차이도 있으므로 체크해줘야함)
private:
	int			m_door_id;
	Door_State	m_state = Door_State::ST_CLOSE;


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

	void init();

	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object();

	bool process_door_event();

	void set_boundingbox_check(bool option) { m_check_bounding_box = option; }

	void Release();
	Door_State get_state() { return m_state; }
};

class Vent : public GameObject {
private:
	int			m_door_id;
	Door_State	m_state = Door_State::ST_CLOSE;

public:		
	mutex*								m_state_lock = nullptr;
	bool								m_check_bounding_box = true;

public:
	Vent();
	Vent(const unsigned int obj_id, Object_Type type, const XMFLOAT3& center, const XMFLOAT3& extents);
	~Vent() = default;

	void		init();

	bool		process_door_event();
	void		set_boundingbox_check(bool option) { m_check_bounding_box = option; }

	void		Release();
	Door_State	get_state() { return m_state; }
};

class ElectronicSystem : public GameObject {
private:
	int			m_system_id = -1;
	bool		m_correct_on_off_switch[ON_OFF_SWITCH]{false};
	bool		m_check_on_off_switch[ON_OFF_SWITCH]{ false };
	bool		m_fixed_system = false;
	ES_State	m_state = ES_State::ES_CLOSE;

public:
	mutex*		m_state_lock = nullptr;
	bool		m_electronic_door_working = false;
public:
	ElectronicSystem();
	ElectronicSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents);
	~ElectronicSystem() = default;

	void Reset();

	void init_electrinic_switch_data(int idx, bool value) { m_correct_on_off_switch[idx] = value; }

	bool Get_On_Off_Switch_Vaild();
	bool Get_On_Off_Switch_Value(int idx) { return m_check_on_off_switch[idx]; }
	bool Get_On_Off_Switch_Correct_Value(int idx) { return m_correct_on_off_switch[idx]; }

	void Set_On_Off_Switch_Value(int idx, bool value) { m_check_on_off_switch[idx] = value; }
	void Set_Close_Electronic_System();

	bool Activate_ElectronicSystem();

	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object(bool value);

	void Release();
	ES_State get_state() { return m_state; }
};

class EscapeSystem : public GameObject {
private:
	int		m_system_id = -1;
	bool	m_activate = false;
	bool	m_working = false;

	mutex* m_state_lock;
public:
	EscapeSystem();
	EscapeSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents);
	~EscapeSystem() = default;

	void init();
	void Activate();
	void Working_Escape();

	bool Is_Activate();
	bool Is_Working_Escape();

	void Release();
	void Update_bounding_box_pos(const XMFLOAT3& pos);
	void Update_Object();
};

class GameItem : public GameObject {
private:
	GAME_ITEM::ITEM m_item_type;
	bool			m_own = false;
	bool			m_show = false;
	
	short			m_item_box_index;

	mutex*			m_state_lock;
public:
	GameItem();
	GameItem(GAME_ITEM::ITEM item_type, const XMFLOAT3& extents);
	~GameItem() = default;

	void				init();

	void				Set_Item_box_index(const unsigned short index) { m_item_box_index = index; }
	void				Set_Item_Type(GAME_ITEM::ITEM item_type) { m_item_type = item_type; }

	short				Get_Item_box_index() const{ return m_item_box_index; }
	GAME_ITEM::ITEM		Get_Item_Type() { return m_item_type; }

	void				Update_bounding_box_pos(const XMFLOAT3& pos);
	bool				Pict_Item();
	void				Update_Object();
	void				Release();
};

class Altar : public GameObject {
private:
	bool	m_is_valid = false;
	mutex*	m_state_lock = nullptr;

	int		m_have_life_chip = 0;
public:
	Altar();
	~Altar() = default;

	void	init();
	void	Set_Valid(bool value);
	void	Add_Life_Chip() { m_have_life_chip++; }

	bool	Get_Valid();
	int		Get_Life_Chip() { return m_have_life_chip; }
	void	Release();
};