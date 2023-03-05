#include "object.h"

GameObject::GameObject()
{

}

GameObject::GameObject(Object_Type type, float width, float height)
{

}

GameObject::~GameObject()
{

}

void GameObject::init(Object_Type type, float width, float height)
{
	m_type = type;
	m_width = width;
	m_height = height;

	Set_BoundingBox();
}

void GameObject::Set_BoundingBox()
{
	m_bounding_box = BoundingOrientedBox{ XMFLOAT3{0.f, 0.f, 0.f}, XMFLOAT3{0.65f, 0.37f, 0.65f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 1.0f} }; // �ӽð����� ������Ʈ�� ���� ���� �ҷ��ͼ� ���� ����
}

BoundingOrientedBox GameObject::Get_BoundingBox()
{
	return m_bounding_box;
}