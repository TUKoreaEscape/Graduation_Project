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

	//std::cout << "m_body : " << m_body << std::endl;
	//std::cout << "m_body_parts : " << m_body_parts << std::endl;
	//std::cout << "m_eyes : " << m_eyes << std::endl;
	//std::cout << "m_gloves : " << m_gloves << std::endl;
	//std::cout << "m_head : " << m_head << std::endl;
	//std::cout << "m_mouthandnoses : " << m_mouthandnoses << std::endl;
	//std::cout << "커스터마이징 정보 로드 완료" << std::endl;
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