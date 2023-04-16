#include "User_Custom.h"
#include "DataBase.h"
Customizing_Info::Customizing_Info()
{
	m_database = new DataBase;
}

Customizing_Info::~Customizing_Info()
{
	delete m_database;
}

void Customizing_Info::Load_Customizing_Data_To_DB(std::wstring id)
{
	// 이쪽에서 db함수 로드
	Custom load_custom = m_database->Load_Customizing(id);

	m_body = load_custom.body;
	m_body_parts = load_custom.body_parts;
	m_eyes = load_custom.eyes;
	m_gloves = load_custom.gloves;
	m_head = load_custom.head;
	m_head_parts = load_custom.head_parts;
	m_mouthandnoses = load_custom.mouthandnoses;
	m_tails = load_custom.tails;
}

void Customizing_Info::Save_Customizing_Data_To_DB(std::wstring id)
{
	Custom save_custom;

	save_custom.body = m_body;
	save_custom.body_parts = m_body_parts;
	save_custom.eyes = m_eyes;
	save_custom.gloves = m_gloves;
	save_custom.head = m_head;
	save_custom.head_parts = m_head_parts;
	save_custom.mouthandnoses = m_mouthandnoses;
	save_custom.tails = m_tails;

	m_database->Save_Customizing(id, save_custom);
	// 이쪽에서 db함수 로드
}