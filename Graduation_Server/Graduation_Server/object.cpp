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

bool ElectronicSystem::Get_On_Off_Switch_Vaild(int idx, bool data[])
{
	for (int i = 0; i < 15; ++i)
	{
		if (m_correct_on_off_switch[i] != data[i])
			return false;
	}
	return true;
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
}

void EscapeSystem::Update_bounding_box_pos(const XMFLOAT3& pos)
{

}

void EscapeSystem::Update_Object()
{

}