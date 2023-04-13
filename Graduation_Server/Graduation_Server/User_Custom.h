#pragma once
class DataBase;

class Customizing_Info {
private:
	int	m_head_part;
	int m_body_part;
	int m_leg_part;

	DataBase* m_database = nullptr;
public:
	Customizing_Info();
	~Customizing_Info();

	void Load_Customizing_Data_To_DB(char* id);
	void Save_Customizing_Data_To_DB(char* id);

	void Set_Head_Custom(int head_part) { m_head_part = head_part; }
	void Set_Body_Custom(int body_part) { m_body_part = body_part; }
	void Set_Leg_Custom(int leg_part) { m_leg_part = leg_part; };

	int Get_Head_Custom() { return m_head_part; }
	int Get_Body_Custom() { return m_body_part; }
	int Get_Leg_Custom() { return m_leg_part; }
};