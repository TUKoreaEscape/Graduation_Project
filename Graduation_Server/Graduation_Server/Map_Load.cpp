#include "Map_Load.h"

Game_Map::Game_Map()
{

}

Game_Map::~Game_Map()
{

}

void Game_Map::map_init()
{
	char map_data;

	fopen_s(&fp, ".txt", "rb");
	while (fscanf_s(fp, "%c", &map_data) != EOF) {

	}
}