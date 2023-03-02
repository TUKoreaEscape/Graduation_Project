#pragma once
#include <fstream>

class Game_Map {
private:
	FILE* fp;
public:
	Game_Map();
	~Game_Map();

	void map_init();
};