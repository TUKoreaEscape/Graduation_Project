#include <iostream>
#include <ws2tcpip.h>
#include <thread>
#include <locale.h>
#include <stdio.h>


#include "EXP_OVER.h"
#include "protocol.h"
using namespace std;

#pragma comment(lib, "WS2_32.LIB")
#define BUF_SIZE 512

const char* SERVER_ADDR = "127.0.0.1";
SOCKET g_s_socket;
thread g_net_recv_Thread;
thread g_net_send_Thread;

void SendPacket(void* packet)
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
//	cout << "받음" << endl;
	switch (ptr[1])
	{
	case SC_PACKET::SC_LOGINOK:
		cout << "로그인에 성공하였습니다!" << endl;
		cout << "===============================================" << endl;
		break;

	case SC_PACKET::SC_LOGINFAIL:
		if (ptr[2] == LOGIN_FAIL_REASON::INVALID_ID)
			cout << "로그인에 실패하였습니다. (사유 : 존재하지 않는 ID 입니다.)" << endl;
		else if (ptr[2] == LOGIN_FAIL_REASON::WRONG_PW)
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

	case SC_PACKET::SC_CREATE_ROOM_OK:
	{
		sc_packet_create_room* packet = reinterpret_cast<sc_packet_create_room*>(ptr);
		cout << packet->room_number << "번방을 생성하였습니다. " << endl;
		break;
	}
	case SC_PACKET::SC_PACKET_ROOM_INFO:
	{
		sc_packet_request_room_info* packet = reinterpret_cast<sc_packet_request_room_info*>(ptr);

		cout << "send_packet size : " << sizeof(ptr) << endl;
		for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i)
			cout << packet->room_info[i].room_number << "번방 ["  << packet->room_info[i].room_name << "] : [" << packet->room_info[i].join_member << "/6]" << endl;

		
		break;
	}
	
	case SC_PACKET::SC_PACKET_JOIN_ROOM_FAIL:
	{
		sc_packet_join_room_fail* packet = reinterpret_cast<sc_packet_join_room_fail*>(ptr);
		cout << "방 접속에 실패하였습니다!" << endl;
		break;
	}

	case SC_PACKET::SC_PACKET_JOIN_ROOM_SUCCESS:
	{
		sc_packet_join_room_success* packet = reinterpret_cast<sc_packet_join_room_success*>(ptr);
		cout << packet->room_number << "번 방에 접속 하였습니다." << endl;
		break;
	}

	case SC_PACKET::SC_PACKET_CHAT:
	{
		sc_packet_chat* packet = reinterpret_cast<sc_packet_chat*>(ptr);
		cout << packet->name << " : " << packet->message << endl;
		break;
	}
	
	case SC_PACKET::SC_USER_UPDATE:
	{
		sc_update_user_packet* packet = reinterpret_cast<sc_update_user_packet*>(ptr);
		cout << packet->data.id << "가 이동 pos : (" << packet->data.position.x << ", " << packet->data.yaw << ", " << packet->data.position.z << ") || vel : (" << packet->data.velocity.x << ", " << packet->data.velocity.y << ", " << packet->data.velocity.z << ") || yaw : " << packet->data.yaw << endl;
		break;
	}

	case SC_PACKET::SC_PACKET_GAME_START:
	{
		cout << "모든 플레이어가 준비되어 게임을 시작합니다." << endl;
		cs_packet_loading_success packet;
		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_PACKET_GAME_LOADING_SUCCESS;

		SendPacket(&packet);
		break;
	}

	}
}

void PacketReassembly(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t make_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (io_byte != 0) {
		if (make_packet_size == 0) {
			make_packet_size = ptr[0];
			if (make_packet_size == 127)
			{
				make_packet_size *= ptr[2];
				make_packet_size += ptr[3];
			}
		
		}
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
			cout << "recv_byte : " << recv_byte << endl;
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

	SendPacket(&packet);
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

	SendPacket(&packet);
}

void Create_Room_Test()
{
	cs_packet_create_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_CREATE_ROOM;

	SendPacket(&packet);
}

void Request_Room_Info_Test()
{
	cs_packet_request_all_room_info packet;
	packet.request_page = 0;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO;

	SendPacket(&packet);
}

void Join_Room_Test()
{
	int room_number;
	cout << "접속할 방 번호를 입력 해 주세요 : ";
	cin >> room_number;

	cs_packet_join_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_JOIN_ROOM;
	packet.room_number = room_number;

	SendPacket(&packet);
}

void Exit_Room_Test()
{
	cs_packet_request_exit_room packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_EXIT_ROOM;
	packet.request_page = 0;

	SendPacket(&packet);
}

void Chat_Test()
{
	cs_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_CHAT;
	cout << "메세지 입력: ";
	fgets(packet.message, 100, stdin);
	packet.room_number = 0;


	SendPacket(&packet);
}

void Move_Test()
{
	cs_packet_move packet;
	
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_MOVE;
	packet.position = { 0,0,0 };
	packet.velocity = { 1,1,1 };
	packet.yaw = 0.3f;

	SendPacket(&packet);
}

void Ready_Test()
{
	cs_packet_ready packet;
	
	packet.size = sizeof(packet);
	packet.type = CS_PACKET::CS_PACKET_READY;
	packet.ready_type = true;

	SendPacket(&packet);
}

void Stress_Test()
{
	while (true)
	{
		cs_packet_move packet;

		packet.size = sizeof(packet);
		packet.type = CS_PACKET::CS_MOVE;
		packet.position = { 0,0,0 };
		packet.velocity = { 1,1,1 };
		packet.yaw = 0.3f;

		SendPacket(&packet);
		this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
void Send_Packet()
{
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::milliseconds(1));
	//	Stress_Test();
	//}

	while (true)
	{
		int test_code = 0;
		cout << "원하는 테스트를 선택하세요." << endl;
		cout << "1. ID 로그인 테스트" << endl;
		cout << "2. ID 생성 테스트" << endl;
		cout << "3. (로그인 후) 방 생성" << endl;
		cout << "4. 생성된 방 모든 정보 요청" << endl;
		cout << "5. 방 접속 테스트" << endl;
		cout << "6. 방 나가기 테스트" << endl;
		cout << "7. 채팅 테스트 (방 접속시에만 가능)" << endl;
		cout << "8. 이동패킷 전송 테스트" << endl;
		cout << "9. Ready, 전원Ready시 게임시작 테스트" << endl;
		cout << "입력 : ";
		cin >> test_code;

		system("cls");
		if (test_code == 1)
			Login_Test();
		else if (test_code == 2)
			CreateID_Test();
		else if (test_code == 3)
			Create_Room_Test();
		else if (test_code == 4)
			Request_Room_Info_Test();
		else if (test_code == 5)
			Join_Room_Test();
		else if (test_code == 6)
			Exit_Room_Test();
		else if (test_code == 7)
			Chat_Test();
		else if (test_code == 8)
			Move_Test();
		else if (test_code == 9)
			Ready_Test();


		this_thread::sleep_for(std::chrono::seconds(1));
		system("cls");
	}
}





int main()
{
	_wsetlocale(LC_ALL, L"korean");
	wcin.imbue(locale("korean"));          //입력시 부분적 적용
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