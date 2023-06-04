#include "IOCP.h"
#include "GameServer.h"
#include "protocol.h"

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

cGameServer* cGameServer::server_instance = NULL;

int main()
{
	_wsetlocale(LC_ALL, L"korean");
	cGameServer& game_server = *cGameServer::GetInstance();
	game_server.init();
	game_server.StartServer();
}