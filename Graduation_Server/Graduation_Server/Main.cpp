#include "IOCP.h"
#include "GameServer.h"
#include "protocol.h"

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

cGameServer* g_server = nullptr;

int main()
{
	g_server = new cGameServer;
	g_server->init();
	g_server->StartServer();

	delete g_server;
}