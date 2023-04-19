#pragma once
#include "stdafx.h"
#include "IOCP.h"
#include "EXPOver.h"

using namespace std;

void C_IOCP::Start_server()
{
	int retcode = 0;
	WSADATA wsa;
	retcode = WSAStartup(MAKEWORD(2, 2), &wsa);
	this->Bind_Socket(SERVER_PORT);
}

void C_IOCP::Close_server()
{

}

void C_IOCP::Bind_Socket(short port_num)
{
	m_listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	bind(m_listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(m_listen_socket, SOMAXCONN);

	m_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listen_socket), m_h_iocp, 0, 0);

	m_client_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char buf_accept[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	*(reinterpret_cast<SOCKET*>(&m_exp_over.m_buf)) = m_client_socket;
	ZeroMemory(&m_exp_over.m_wsa_over, sizeof(m_exp_over.m_wsa_over));
	m_exp_over.m_comp_op = OP_TYPE(OP_ACCEPT);

	AcceptEx(m_listen_socket, m_client_socket, buf_accept, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &m_exp_over.m_wsa_over);

	std::cout << "Server Start!!! \n";
}


void C_IOCP::Disconnect()
{

}