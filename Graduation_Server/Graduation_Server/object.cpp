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
	BoundingOrientedBox bounding_box(center, extents, orientation);
	Set_BoundingBox(bounding_box);
}

void GameObject::Set_BoundingBox(const BoundingOrientedBox& box)
{
	m_bounding_box = box;
}

void GameObject::Update_bounding_box_pos(const XMFLOAT3& pos)
{
	m_bounding_box.Center = pos;
}

void GameObject::Update_bounding_box_rotate(const float yaw)
{
	float radian = XMConvertToRadians(yaw);

	XMFLOAT4 calculate{};
	XMStoreFloat4(&calculate, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = calculate;
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
	cGameServer& server = *cGameServer::GetInstance();
	return m_orientation;
}

Door::Door()
{
	if (m_state_lock == nullptr)
		m_state_lock = new mutex;
}

Door::Door(const unsigned int door_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_door_id = door_id;
	m_type = type;
	m_pos = center;
	m_extents = extents;
	m_state = ST_CLOSE;
	m_bounding_box = BoundingOrientedBox{ center, extents, XMFLOAT4(0,0,0,1) };
	if(m_state_lock == nullptr)
		m_state_lock = new mutex;
}

bool Door::process_door_event()
{
	m_state_lock->lock();
	if (m_state == ST_CLOSE && m_door_open_start == false && m_door_close_start == false)
	{
		m_state = ST_OPEN;
		m_door_open_start_time = chrono::system_clock::now();
		m_door_open_start = true;
		m_state_lock->unlock();
	}

	else if (m_state == ST_OPEN && m_door_close_start == false && m_door_open_start == false)
	{
		m_state = ST_CLOSE;
		m_door_close_start = true;
		m_door_close_start_time = chrono::system_clock::now();
		m_state_lock->unlock();
	}
	return true;
}

void Door::Update_bounding_box_pos(const XMFLOAT3& pos)
{
	m_bounding_box.Center = pos;
}

void Door::Update_Object()
{

}

void Door::Release()
{
	if(m_state_lock != nullptr)
		delete m_state_lock;
}

ElectronicSystem::ElectronicSystem()
{

}

ElectronicSystem::ElectronicSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_system_id = obj_id;
	m_type = type;
	m_pos = center;
	m_extents = extents;
	m_bounding_box = BoundingOrientedBox{ center, extents, XMFLOAT4(0, 0, 0, 1) };
	if (m_state_lock == nullptr)
		m_state_lock = new mutex;
}

void ElectronicSystem::Reset()
{
	m_state_lock->lock();
	m_state = ES_CLOSE;
	m_fixed_system = false;
	m_state_lock->unlock();
	for (int i = 0; i < ON_OFF_SWITCH; ++i) {
		m_check_on_off_switch[i] = false;
		m_correct_on_off_switch[i] = false;
	}
}

void ElectronicSystem::Update_Object(bool value)
{
	m_state_lock->lock();
	if (value == true)
		m_state = ES_OPEN;
	else
		m_state = ES_CLOSE;
	m_state_lock->unlock();
}

void ElectronicSystem::Update_bounding_box_pos(const XMFLOAT3& pos)
{

}

bool ElectronicSystem::Get_On_Off_Switch_Vaild()
{
	for (int i = 0; i < ON_OFF_SWITCH; ++i)
	{
		if (m_correct_on_off_switch[i] != m_check_on_off_switch[i])
			return false;
	}
	return true;
}

bool ElectronicSystem::Activate_ElectronicSystem()
{
	m_state_lock->lock();
	if (Get_On_Off_Switch_Vaild() == true)
	{
		m_fixed_system = true;
		m_state_lock->unlock();
		return true;
	}
	else {
		m_state_lock->unlock();
		return false;
	}
}

void ElectronicSystem::Release()
{
	delete m_state_lock;
}

EscapeSystem::EscapeSystem()
{
}

EscapeSystem::EscapeSystem(const unsigned int obj_id, Object_Type type, XMFLOAT3 center, XMFLOAT3 extents)
{
	m_system_id = obj_id;
	m_type = type;
	m_pos = center;
	m_extents = extents;
	m_bounding_box = BoundingOrientedBox{ center, extents, XMFLOAT4(0,0,0,1) };
	m_state_lock = new mutex;
}

void EscapeSystem::Release()
{
	if(m_state_lock != nullptr)
		delete m_state_lock;
}

void EscapeSystem::init()
{
	m_activate = false;
	m_working = false;
}

void EscapeSystem::Activate()
{
	if (m_activate == false)
		m_activate = true;
}

void EscapeSystem::Working_Escape()
{
	if (m_working == false)
		m_working = true;
}

bool EscapeSystem::Is_Activate()
{
	return m_activate;
}

bool EscapeSystem::Is_Working_Escape()
{
	return m_working;
}

void EscapeSystem::Update_bounding_box_pos(const XMFLOAT3& pos)
{

}

void EscapeSystem::Update_Object()
{

}

GameItem::GameItem()
{
	m_state_lock = new mutex;
}

GameItem::GameItem(GAME_ITEM::ITEM item_type, const XMFLOAT3& extents)
{
	m_item_type = item_type;
	m_state_lock = new mutex;
}

void GameItem::init()
{
	m_own = false;
	m_item_type = GAME_ITEM::ITEM_NONE;
	m_item_box_index = -1;
}

void GameItem::Update_bounding_box_pos(const XMFLOAT3& pos)
{
	m_bounding_box.Center = pos;
}

void GameItem::Update_Object()
{

}

bool GameItem::Pict_Item()
{
	m_state_lock->lock();
	if (m_show == true)
	{
		m_show = false;
		m_state_lock->unlock();
		return true;
	}
	m_state_lock->unlock();
	return false;
}

void GameItem::Release()
{
	delete m_state_lock;
}


Altar::Altar()
{
	
}

void Altar::init()
{
	if(m_state_lock == nullptr)
		m_state_lock = new mutex;
	m_is_valid = false;
	m_have_life_chip = 0;
}

void Altar::Release()
{
	if(m_state_lock != nullptr)
		delete m_state_lock;
}

void Altar::Set_Valid(bool value)
{
	m_state_lock->lock();
	m_is_valid = true;
	m_state_lock->unlock();
}

bool Altar::Get_Valid()
{
	return m_is_valid;
}