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

void Customizing_Info::Load_Customizing_Data_To_DB(char* id)
{
	// ���ʿ��� db�Լ� �ε�
}

void Customizing_Info::Save_Customizing_Data_To_DB(char* id)
{
	// ���ʿ��� db�Լ� �ε�
}