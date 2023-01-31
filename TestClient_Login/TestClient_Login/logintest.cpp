#include <iostream>
#include <ws2tcpip.h>
#include "EXP_OVER.h"
#include "protocol.h"
using namespace std;
#pragma comment(lib, "WS2_32.LIB")

const char* SERVER_ADDR = "127.0.0.1";
SOCKET g_s_socket;

void SendPacket(int cl, void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];
	EXP_OVER* over = new EXP_OVER;
	over->m_comp_op = OP_SEND;
	memcpy(over->m_buf, packet, psize);
	ZeroMemory(&over->m_wsa_over, sizeof(over->m_wsa_over));
	over->m_wsa_buf.buf = reinterpret_cast<CHAR*>(over->m_buf);
	over->m_wsa_buf.len = psize;
	int ret = WSASend(g_s_socket, &over->m_wsa_buf, 1, NULL, 0, &over->m_wsa_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			;
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

int main()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAdata;

	if (WSAStartup(MAKEWORD(2, 2), &WSAdata) != 0)
		return 1;

	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	cout << "서버에 연결완료" << endl;
	while (true)
	{
		cs_packet_login packet;
		cout << "ID 입력 : ";
		cin >> packet.id;

		cout << "PW 입력 : ";
		cin >> packet.pass_word;

		packet.type = CS_PACKET::CS_LOGIN;
		packet.size = sizeof(cs_packet_login);

		SendPacket(0, &packet);

	}
}