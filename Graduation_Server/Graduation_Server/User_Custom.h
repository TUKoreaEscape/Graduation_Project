#pragma once
#include <string>

class DataBase;

const std::string Bodies[6]{ "MainBody01","MainBody02", "MainBody03", "MainBody04", "MainBody05", "MainBody06" };
const std::string Bodyparts[6]{ "Bodypart02","Bodypart04","Bodypart05","Bodypart06","Bodypart07","Bodypart08" };
const std::string Eyes[7]{ "Eye04","Eye05","Eye06","Eye08", "Eye10","Eye11","Eye12" };
const std::string Gloves[6]{ "Glove01","Glove02","Glove04","Glove06","Glove09","Glove10" };
const std::string MouthandNoses[7]{ "Mouth02", "Mouth03", "Mouth05", "Mouth15", "Nose10", "Nose11", "Nose13" };
const std::string Head[21]{ "Comb14", "Ear02", "Ear03", "Ear04", "Ear05", "Eye03", "Eye07", "Eye13", "Eye14", "Grass15", "Hair06", "Hair09", "Hat16", "Hat17", "Hat19", "Hat20", "Hat21", "Horn10", "Horn11", "Horn12", "Horn13" };

enum PLAYERPARTS {
	PLAYER = 0, OTHER1, OTHER2, OTHER3, OTHER4, OTHER5
};

enum BODIES {
	MAINBODY01 = 0, MAINBODY02,
	MAINBODY03, MAINBODY04,
	MAINBODY05, MAINBODY06
};

enum BODYPARTS {
	BODYPART01 = 0, BODYPART02, BODYPART03, BODYPART04,
	BODYPART05, BODYPART06
};

enum EYES {
	EYE01 = 0, EYE02, EYE03, EYE04, EYE05,
	EYE06, EYE07
};

enum GLOVES {
	GLOVE01 = 0, GLOVE02, GLOVE03, GLOVE04, GLOVE05,
	GLOVE06
};

enum MOUTHANDNOSES {
	MOUTH01 = 0, MOUTH02, MOUTH03, MOUTH04, MOUTH05,
	MOUTH06, MOUTH07
};

enum HEADS {
	HEAD01 = 0, HEAD02, HEAD03, HEAD04, HEAD05,
	HEAD06, HEAD07, HEAD08, HEAD09, HEAD10,
	HEAD11, HEAD12, HEAD13, HEAD14, HEAD15,
	HEAD16, HEAD17, HEAD18, HEAD19, HEAD20,
	HEAD21
};

struct Custom {
	HEADS			head;
	BODIES			body;
	BODYPARTS		body_parts;
	EYES			eyes;
	GLOVES			gloves;
	MOUTHANDNOSES	mouthandnoses;
};

class Customizing_Info {
private:
	HEADS			m_head;
	BODIES			m_body;
	BODYPARTS		m_body_parts;
	EYES			m_eyes;
	GLOVES			m_gloves;
	MOUTHANDNOSES	m_mouthandnoses;

	DataBase* m_database = nullptr;
public:
	Customizing_Info();
	~Customizing_Info();

	void Load_Customizing_Data_To_DB(Custom& data);
	void Save_Customizing_Data_To_DB(std::wstring id);

	void Set_Head_Custom(HEADS head) { m_head = head; }
	void Set_Body_Custom(BODIES body) { m_body = body; }
	void Set_Body_Part_Custom(BODYPARTS bodys_part) { m_body_parts = bodys_part; }
	void Set_Eyes_Custom(EYES eyes) { m_eyes = eyes; }
	void Set_Gloves_Custom(GLOVES gloves) { m_gloves = gloves; }
	void Set_Mouthandnoses_Custom(MOUTHANDNOSES mouthandnoses) { m_mouthandnoses = mouthandnoses; }

	HEADS			Get_Head_Custom() { return m_head; }
	BODIES			Get_Body_Custom() { return m_body; }
	BODYPARTS		Get_Body_Part_Custom() { return m_body_parts; }
	EYES			Get_Eyes_Custom() { return m_eyes; }
	GLOVES			Get_Gloves_Custom() { return m_gloves; }
	MOUTHANDNOSES	Get_Mouthandnoses_Custom() { return m_mouthandnoses; }
};