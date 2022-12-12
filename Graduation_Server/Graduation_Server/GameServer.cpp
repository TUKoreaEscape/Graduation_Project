#include "stdafx.h"
#include <bitset>
#include "GameServer.h"

cGameServer::cGameServer()
{

}

cGameServer::~cGameServer()
{

}

void cGameServer::init()
{
	// 서버 시작시 기본 초기화는 여기서 해줄 예정!
	//m_voice_chat = new Voice_Chat;
	//m_voice_chat->Init();

}

void cGameServer::StartServer()
{
	C_IOCP::Start_server();

	for (int i = 0; i < 8; ++i)
		m_worker_threads.emplace_back(std::thread(&cGameServer::WorkerThread, this));
	m_timer_thread = std::thread{ &cGameServer::m_timer_thread, this };

	for (auto& worker : m_worker_threads)
		worker.join();
	m_timer_thread.join();
}

void cGameServer::WorkerThread()
{
	while (true)
	{
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(C_IOCP::m_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		if (FALSE == ret)
		{
			int err_no = WSAGetLastError();
			// 이쪽은 에러탐색부분!
			std::cout << "ID:" << client_id << " Error!";
			if (err_no == ERROR_INVALID_HANDLE)
				std::cout << "Handle Error" << std::endl;

			std::cout << err_no << std::endl;

			WCHAR* lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, err_no,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, 0);
			std::wcout << lpMsgBuf << std::endl;

			LocalFree(lpMsgBuf);
			Disconnect(client_id);
			
			if (exp_over->m_comp_op == OP_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->m_comp_op) {
		case OP_RECV:
			if (num_byte == 0)
				Disconnect(client_id);
			Recv(exp_over, client_id, num_byte);
			break;

		case OP_SEND:
			if (num_byte != exp_over->m_wsa_buf.len) {
				std::cout << "send_error" << std::endl;
				Disconnect(client_id);
			}
			delete exp_over;
			break;

		case OP_ACCEPT:
			Accept(exp_over);
			break;
		}
	}
}

void cGameServer::Accept(EXP_OVER* exp_over)
{
	std::cout << "Accept Completed \n";
	SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->m_buf));
	unsigned int new_id = get_new_id();
	if (-1 == new_id)
		std::cout << "Maxumum user overflow. Accept aborted \n";
	else
	{
		cout << "Accept Client! \n";
		CLIENT& cl = m_clients[new_id];
		cl.set_prev_size(0);
	}
}

void cGameServer::Recv(EXP_OVER* exp_over, const unsigned int user_id, const DWORD num_byte)
{
	CLIENT& cl = m_clients[user_id];

	int remain_data = num_byte + cl.return_prev_size();
	unsigned char* packet_start = exp_over->m_buf;
	int packet_size = packet_start[0];

	while (packet_size <= remain_data)
	{
		ProcessPacket(user_id, packet_start);
		remain_data -= packet_size;
		packet_start += packet_size;

		if (remain_data > 0)
			packet_size = packet_start[0];
		else
			break;
	}

	if (0 < remain_data)
	{
		cl.set_prev_size(remain_data);
		memcpy(&exp_over->m_buf, packet_start, remain_data);
	}
	cl.do_recv();
	// and send clients sending
}

void cGameServer::ProcessPacket(const unsigned int user_id, unsigned char* p) // 패킷구분 후 처리
{
	unsigned char p_type = p[1];

	switch (p_type) // 패킷 타입별로 처리할 공간
	{
	case CS_PACKET::CS_LOGIN:
		// 로그인 처리
		break;

	case CS_PACKET::CS_MOVE:

		break;

	case CS_PACKET::CS_CHAT:

		break;
	}
}

void cGameServer::Disconnect(const unsigned int _user_id)
{
	;
}

int cGameServer::get_new_id()
{
	static int g_id = 0;

	for (int i = 0; i < MAX_USER; ++i)
	{
		m_clients[i].set_state_lock();
		if (ST_FREE == m_clients[i].get_state())
		{
			m_clients[i].set_state(ST_ACCEPT);
			m_clients[i].set_state_unlock();
			return i;
		}
		m_clients[i].set_state_unlock();
	}

	std::cout << "Maximum Number of Clients Overflow! \n";
	return -1;
}