#include <iostream>
#include <ws2tcpip.h>
#include <thread>
#include "EXP_OVER.h"
#include "protocol.h"
using namespace std;
#pragma comment(lib, "WS2_32.LIB")
#define BUF_SIZE 512

const char* SERVER_ADDR = "127.0.0.1";
SOCKET g_s_socket;
thread g_net_recv_Thread;
thread g_net_send_Thread;

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

void ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	case SC_PACKET::SC_LOGINOK:
		cout << "로그인에 성공하였습니다!" << endl;
		cout << "===============================================" << endl;
		break;

	case SC_PACKET::SC_LOGINFAIL:
		if (ptr[2] == 0)
			cout << "로그인에 실패하였습니다. (사유 : 존재하지 않는 ID 입니다.)" << endl;
		else if (ptr[2] == 2)
			cout << "로그인에 실패하였습니다. (사유 : PW가 틀립니다.)" << endl;
		cout << "===============================================" << endl;
		break;

	case SC_PACKET::SC_CREATE_ID_OK:
		cout << "ID 생성에 성공하였습니다!" << endl;
		cout << "===============================================" << endl;
		break;

	case SC_PACKET::SC_CREATE_ID_FAIL:
		if (ptr[2] == 0)
			cout << "ID 생성에 실패하였습니다. (사유 : 이미 같은 ID가 존재합니다.)" << endl;
		else if (ptr[2] == 2)
			cout << "ID 생성에 실패하였습니다. (사유 : DB 연결 에러입니다.)" << endl;
		cout << "===============================================" << endl;
		break;
	}
}

void PacketReassembly(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t make_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (io_byte != 0) {
		if (make_packet_size == 0)
			make_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= make_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, make_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ZeroMemory(packet_buffer, BUF_SIZE);
			ptr += make_packet_size - saved_packet_size;
			io_byte -= make_packet_size - saved_packet_size;
			//cout << "io byte - " << io_byte << endl;
			make_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void RecvPacket()
{
	while (true)
	{
		char buf[BUF_SIZE] = { 0 };
		WSABUF wsabuf{ BUF_SIZE, buf };
		DWORD recv_byte{ 0 }, recv_flag{ 0 };

		if (WSARecv(g_s_socket, &wsabuf, 1, &recv_byte, &recv_flag, nullptr, nullptr) == SOCKET_ERROR)
			;

		if (recv_byte > 0) {
			//cout << "recv_byte : " << recv_byte << endl;
			PacketReassembly(wsabuf.buf, recv_byte);
		}
	}
}

void Login_Test()
{
	cs_packet_login packet;
	cout << "===============================================" << endl;
	cout << "ID 입력 : ";
	cin >> packet.id;

	cout << "PW 입력 : ";
	cin >> packet.pass_word;
	packet.type = CS_PACKET::CS_LOGIN;
	packet.size = sizeof(packet);

	SendPacket(0, &packet);
}

void CreateID_Test()
{
	cs_packet_create_id packet;
	cout << "===============================================" << endl;
	cout << "새로운 ID 입력 : ";
	cin >> packet.id;

	cout << "새로운 PW 입력 : ";
	cin >> packet.pass_word;

	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_CREATE_ID;

	SendPacket(0, &packet);
}

void Send_Packet()
{
	while (true)
	{
		int test_code = 0;
		cout << "원하는 테스트를 선택하세요." << endl;
		cout << "1. ID 로그인 테스트" << endl;
		cout << "2. ID 생성 테스트" << endl;
		cout << "입력 : ";
		cin >> test_code;

		system("cls");
		if (test_code == 1)
			Login_Test();
		else if (test_code == 2)
			CreateID_Test();

		this_thread::sleep_for(std::chrono::seconds(1));
	}
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
	g_net_recv_Thread = thread{ RecvPacket };
	g_net_send_Thread = thread{ Send_Packet };
		
	g_net_recv_Thread.join();
	g_net_send_Thread.join();

}