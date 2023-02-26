#include "stdafx.h"
#include <bitset>
#include "GameServer.h"

cGameServer::cGameServer()
{

}

cGameServer::~cGameServer()
{
	delete m_voice_chat;
	delete m_room_manager;
	delete m_database;
}

void cGameServer::init()
{
	// ���� ���۽� �⺻ �ʱ�ȭ�� ���⼭ ���� ����!
	m_database = new DataBase;
	
	// �ʱ�ȭ�Լ� �� �������! �Ƹ��� ��

	m_voice_chat = new Voice_Chat;
	m_voice_chat->Init();

	m_room_manager = new RoomManager;
	m_room_manager->init();
}

void cGameServer::StartServer()
{
	C_IOCP::Start_server();

	// ������ ���� �� �ε��� �κ�
	for (int i = 0; i < 8; ++i)
		m_worker_threads.emplace_back(std::thread(&cGameServer::WorkerThread, this));

	for (auto& worker : m_worker_threads)
		worker.join();
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
			// ������ ����Ž���κ�!
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
			//if (num_byte == 0)
			//	//Disconnect(client_id);
			Recv(exp_over, client_id, num_byte);
			cout << client_id << "_send \n";
			break;

		case OP_SEND:
			if (num_byte != exp_over->m_wsa_buf.len) {
				std::cout << "send_error" << std::endl;
				//Disconnect(client_id);
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
	if(DEBUG)
		std::cout << "Accept Completed \n";

	unsigned int new_id = get_new_id();
	if (-1 == new_id) {}
	else
	{
		if(DEBUG)
			cout << "Accept Client! new_id : " << new_id << endl;
		SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->m_buf));
		m_clients[new_id]._socket = c_socket;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), m_h_iocp, new_id, 0);
		m_clients[new_id].do_recv();

		ZeroMemory(&exp_over->m_wsa_over, sizeof(exp_over->m_wsa_over));
		c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		*(reinterpret_cast<SOCKET*>(exp_over->m_buf)) = c_socket;
		AcceptEx(C_IOCP::m_listen_socket, c_socket, exp_over->m_buf + 8, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->m_wsa_over);
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

wstring cGameServer::stringToWstring(const std::string& t_str) // string -> wstring ��ȯ
{
	typedef codecvt_utf8<wchar_t>  convert_type;
	wstring_convert<convert_type, wchar_t> converter;

	return converter.from_bytes(t_str);

}

void cGameServer::Send(EXP_OVER* exp_over)
{

}

void cGameServer::Disconnect(const unsigned int _user_id) // Ŭ���̾�Ʈ ������ Ǯ����(������ �������� or ��������� ���)
{
	CLIENT& cl = m_clients[_user_id];

	// ���⼭ �ʱ�ȭ

	m_clients[_user_id]._state_lock.lock();
	closesocket(m_clients[_user_id]._socket);
	m_clients[_user_id].set_state(ST_FREE);
	m_clients[_user_id].set_login_state(N_LOGIN);
	m_clients[_user_id]._state_lock.unlock();
}


//============================================================================
// ��Ŷ ������ ó�����ִ� ����
//============================================================================
void cGameServer::ProcessPacket(const unsigned int user_id, unsigned char* p) // ��Ŷ���� �� ó��
{
	unsigned char packet_type = p[1];

	switch (packet_type) // ��Ŷ Ÿ�Ժ��� ó���� ����
	{
	case CS_PACKET::CS_LOGIN:
	{
		// �α��� ó��
		Process_User_Login(user_id, p);
		break;
	}

	case CS_PACKET::CS_CREATE_ID:
	{	
		Process_Create_ID(user_id, p);
		break;
	}

	case CS_PACKET::CS_MOVE:
	{
		if(Y_LOGIN == m_clients[user_id].get_login_state())
			Process_Move(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_CHAT:
	{
		Process_Chat(user_id, p);
		break;
	}
	case CS_PACKET::CS_PACKET_CREATE_ROOM:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state())
			Process_Create_Room(user_id);
		break;
	}

	case CS_PACKET::CS_PACKET_JOIN_ROOM:
	{

		break;
	}

	case CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO:
	{
		Process_Request_Room_Info(user_id, p);
		break;
	}

	}
}
//============================================================================
// ������ ��Ŷ ó���ϴ� ����
//============================================================================
void cGameServer::Process_Create_ID(int c_id, void* buff) // ��û���� ID������Ŷ ó��
{
	int reason = 0;

	cs_packet_create_id* packet = reinterpret_cast<cs_packet_create_id*>(buff);

	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->create_id(stringToWstring(stringID), stringToWstring(stringPW));

	cout << "reason : " << reason << endl;
	if (reason == 1) // id ���� ����
		send_create_id_ok_packet(c_id);
	else // id ���� ����
		send_create_id_fail_packet(c_id, reason);
}

void cGameServer::Process_Move(const int user_id, void* buff) // ��û���� ĳ���� �̵��� ó��
{
	cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buff);

	for (auto ptr = m_clients[user_id].view_list.begin(); ptr != m_clients[user_id].view_list.end(); ++ptr)
		send_move_packet(*ptr, packet->pos);
}

void cGameServer::Process_Chat(const int user_id, void* buff)
{
	cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buff);
	char mess[256];

	strcpy_s(mess, packet->message);

	// ���� �濡 �ִ� �������׸� �޼��� ���� ����
	m_clients[user_id]._room_list_lock.lock();
	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
		send_chat_packet(*ptr, user_id, mess);
	m_clients[user_id]._room_list_lock.unlock();
}

void cGameServer::Process_Create_Room(const unsigned int _user_id) // ��û���� ���ο� �� ����
{
	sc_packet_create_room packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ROOM_OK;
	packet.room_number = m_room_manager->Create_room(_user_id);

	m_clients[_user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::Process_Request_Room_Info(const int user_id, void* buff)
{
	cs_packet_request_all_room_info* packet = reinterpret_cast<cs_packet_request_all_room_info*>(buff);

	sc_packet_request_room_info send_packet;
	
	send_packet.size = sizeof(send_packet);
	send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
	
	Room get_room_info;
	int room_number;
	for (int i = 0; i < 10; ++i) // 1�������� 10���� ���� ���δٰ� ����, �������� ������ �غ���
	{

			room_number = i + (packet->request_page * 10);
			get_room_info = m_room_manager->Get_Room_Info(room_number);
			send_packet.room_info[room_number].state = get_room_info._room_state;
			//get_room_info.Get_Room_Name(send_packet.room_info[room_number].room_name, MAX_NAME_SIZE);
			send_packet.room_info[room_number].room_number = room_number;
			send_packet.room_info[room_number].join_member = get_room_info.Get_Number_of_users();
	}

	m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
}

void cGameServer::Process_User_Login(int c_id, void* buff) // �α��� ��û
{
	int reason = 0;

	cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buff);
	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->check_login(stringToWstring(stringID), stringToWstring(stringPW));
	if (reason == 1) // reason 0 : id�� �������� ���� / reason 1 : ���� / reason 2 : pw�� Ʋ��
	{
		send_login_ok_packet(c_id);
		m_clients[c_id].set_login_state(Y_LOGIN);
	}
	else
	{
		if (reason == 0)
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::INVALID_ID);
		else
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::WRONG_PW);
	}
}
//============================================================================
// �������� ������ ��Ŷ �Լ��� 
//============================================================================
void cGameServer::send_chat_packet(int user_id, int my_id, char* mess)
{
	sc_packet_chat packet;
	packet.id = my_id;
	packet.type = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_CHAT;
	strcpy_s(packet.message, mess);
	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_login_fail_packet(int user_id, LOGIN_FAIL_REASON::TYPE reason)
{
	sc_packet_login_fail packet;
	packet.type = SC_PACKET::SC_LOGINFAIL;
	packet.size = sizeof(sc_packet_login_fail);
	packet.reason = reason;
	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_login_ok_packet(int user_id)
{
	sc_packet_login_ok packet;
	packet.id = user_id;
	packet.size = sizeof(sc_packet_login_ok);
	packet.type = SC_PACKET::SC_LOGINOK;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_ok_packet(int user_id)
{
	sc_packet_create_id_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ID_OK;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_create_id_fail_packet(int user_id, char reason)
{
	sc_packet_create_id_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ID_FAIL;
	packet.reason = reason;

	m_clients[user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::send_move_packet(int user_id, Position pos)
{
	sc_packet_move packet;

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_MOVING;
	
	m_clients[user_id].do_send(sizeof(packet), &packet);

}


int cGameServer::get_new_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		m_clients[i]._state_lock.lock();
		if (ST_FREE == m_clients[i].get_state())
		{
			m_clients[i].set_state(ST_ACCEPT);
			m_clients[i]._state_lock.unlock();
			cout << "gen_id : " << i << endl;
			return i;
		}
		m_clients[i]._state_lock.unlock();
	}

	std::cout << "Maximum Number of Clients Overflow! \n";
	return -1;
}
//============================================================================
