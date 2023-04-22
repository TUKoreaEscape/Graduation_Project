#include "User_Custom.h"
#include "DataBase.h"
Customizing_Info::Customizing_Info()
{
	//m_database = new DataBase;
}

Customizing_Info::~Customizing_Info()
{
	//delete m_database;
}

void Customizing_Info::Load_Customizing_Data_To_DB(Custom& data)
{
	m_body = data.body;
	m_body_parts = data.body_parts;
	m_eyes = data.eyes;
	m_gloves = data.gloves;
	m_head = data.head;
	m_mouthandnoses = data.mouthandnoses;
}

void Customizing_Info::Save_Customizing_Data_To_DB(std::wstring id)
{
	Custom save_custom;

	save_custom.body = m_body;
	save_custom.body_parts = m_body_parts;
	save_custom.eyes = m_eyes;
	save_custom.gloves = m_gloves;
	save_custom.head = m_head;
	save_custom.mouthandnoses = m_mouthandnoses;

	m_database->Save_Customizing(id, save_custom);
	// 이쪽에서 db함수 로드
}