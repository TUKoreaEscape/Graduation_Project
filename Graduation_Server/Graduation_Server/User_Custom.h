#pragma once
class DataBase;

const std::string Bodies[6]{ "MainBody01","MainBody02", "MainBody03", "MainBody04", "MainBody05", "MainBody06" };
const std::string Bodyparts[10]{ "Bodypart01","Bodypart02","Bodypart03","Bodypart04","Bodypart05","Bodypart06","Bodypart07","Bodypart08","Bodypart09","Bodypart10" };
const std::string Eyes[11]{ "Eye01","Eye02","Eye04","Eye05","Eye06","Eye08", "Eye09","Eye10","Eye11","Eye12","Eye15" };
const std::string Gloves[10]{ "Glove01","Glove02","Glove03","Glove04","Glove05","Glove06","Glove07","Glove08","Glove09","Glove10" };
const std::string Headparts[4]{ "Ear01", "Hair07", "Hair08", "Hat18" };
const std::string MouthandNoses[15]{ "Mouth01","Mouth02", "Mouth03", "Mouth04", "Mouth05", "Mouth06", "Mouth07", "Mouth08", "Mouth09", "Mouth15", "Nose10", "Nose11", "Nose12", "Nose13", "Nose14" };
const std::string Tails[8]{ "Tails01","Tails02", "Tails03", "Tails04", "Tails05", "Tails06", "Tails07", "Tails08" };
const std::string Head[21]{ "Comb14", "Ear02", "Ear03", "Ear04", "Ear05", "Eye03", "Eye07", "Eye13", "Eye14", "Grass15", "Hair06", "Hair09", "Hat16", "Hat17", "Hat19", "Hat20", "Hat21", "Horn10", "Horn11", "Horn12", "Horn13" };

enum BODIES {
	MAINBODY01 = 0, MAINBODY02,
	MAINBODY03, MAINBODY04,
	MAINBODY05, MAINBODY06
};

enum BODYPARTS {
	BODYPART01 = 0, BODYPART02, BODYPART03, BODYPART04,
	BODYPART05, BODYPART06, BODYPART07, BODYPART08,
	BODYPART09, BODYPART10
};

enum EYES {
	EYE01 = 0, EYE02, EYE04, EYE05, EYE06,
	EYE08, EYE09, EYE10, EYE11, EYE12, EYE15
};

enum GLOVES {
	GLOVE01 = 0, GLOVE02, GLOVE03, GLOVE04, GLOVE05,
	GLOVE06, GLOVE07, GLOVE08, GLOVE09, GLOVE10
};

enum HEADPARTS {
	EAR01 = 0, HAIR07, HAIR08, HAT18
};

enum MOUTHANDNOSES {
	MOUTH01 = 0, MOUTH02, MOUTH03, MOUTH04, MOUTH05,
	MOUTH06, MOUTH07, MOUTH08, MOUTH09, MOUTH15,
	NOSES11, NOSES12, NOSES13, NOSES14, NOSES15
};

enum TAILS {
	TAIL01 = 0, TAIL02, TAIL03, TAIL04,
	TAIL05, TAIL06, TAIL07, TAIL08
};

enum HEADS {
	COMB14 = 0, EAR02, EAR03, EAR04, EAR05, EYE03, EYE07,
	EYE13, EYE14, GRASS15, HAIR06, HAIR09, HAT16, HAT17, HAT19,
	HAT20, HAT21, HORN10, HORN11, HORN12, HORN13
};

class Customizing_Info {
private:
	HEADS			m_head;
	HEADPARTS		m_head_parts;
	BODIES			m_body;
	BODYPARTS		m_body_parts;
	EYES			m_eyes;
	GLOVES			m_gloves;
	MOUTHANDNOSES	m_mouthandnoses;
	TAILS			m_tails;

	DataBase* m_database = nullptr;
public:
	Customizing_Info();
	~Customizing_Info();

	void Load_Customizing_Data_To_DB(char* id);
	void Save_Customizing_Data_To_DB(char* id);

	void Set_Head_Custom(HEADS head) { m_head = head; }
	void Set_Head_Part_Custom(HEADPARTS head_part) { m_head_parts = head_part; }
	void Set_Body_Custom(BODIES body) { m_body = body; }
	void Set_Body_Part_Custom(BODYPARTS bodys_part) { m_body_parts = bodys_part; }
	void Set_Eyes_Custom(EYES eyes) { m_eyes = eyes; }
	void Set_Gloves_Custom(GLOVES gloves) { m_gloves = gloves; }
	void Set_Mouthandnoses_Custom(MOUTHANDNOSES mouthandnoses) { m_mouthandnoses = mouthandnoses; }
	void Set_Tail_Custom(TAILS tails) { m_tails = tails; }

	HEADS			Get_Head_Custom() { return m_head; }
	HEADPARTS		Get_Head_Part_Custom() { return m_head_parts; }
	BODIES			Get_Body_Custom() { return m_body; }
	BODYPARTS		Get_Body_Part_Custom() { return m_body_parts; }
	EYES			Get_Eyes_Custom() { return m_eyes; }
	GLOVES			Get_Gloves_Custom() { return m_gloves; }
	MOUTHANDNOSES	Get_Mouthandnoses_Custom() { return m_mouthandnoses; }
	TAILS			Get_Tails_Custom() { return m_tails; }
};