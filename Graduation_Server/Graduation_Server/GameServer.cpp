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
	m_room_manager->init_object();

	m_session_timer.reset();
}

void cGameServer::StartServer()
{
	C_IOCP::Start_server();

	// ������ ���� �� �ε��� �κ�
	for (int i = 0; i < 12; ++i)
		m_worker_threads.emplace_back(std::thread(&cGameServer::WorkerThread, this));

	m_timer_thread.emplace_back(std::thread(&cGameServer::Update_Session, this));
	for (auto& worker : m_worker_threads)
		worker.join();
	
	for (auto& timer_worker : m_timer_thread)
		timer_worker.join();
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
			cout << "disconnect ID : " << client_id << endl;
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
				delete exp_over;
				continue;
			}
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

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), C_IOCP::m_h_iocp, new_id, 0);
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
	if (cl.get_join_room_number() != -1) {
		Room& rl = *m_room_manager->Get_Room_Info(cl.get_join_room_number());
		rl.Exit_Player(_user_id);	
	}
	cl._state_lock.lock();
	cl.set_state(CLIENT_STATE::ST_FREE);
	cl.set_login_state(N_LOGIN);
	cl.set_id(-1);
	cl._state_lock.unlock();
	closesocket(cl._socket);
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
		if(Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_INGAME)
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
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_LOBBY) // �α����ϰ� �κ� �������� �� ���� ����
			Process_Create_Room(user_id);
		break;
	}

	case CS_PACKET::CS_PACKET_JOIN_ROOM:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_LOBBY)
			Process_Join_Room(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_READY:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_GAMEROOM)
			Process_Ready(user_id, p);
		break;
	}
	
	case CS_PACKET::CS_PACKET_EXIT_ROOM:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_GAMEROOM)
			Process_Exit_Room(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_REQUEST_ROOM_INFO:
	{
		Process_Request_Room_Info(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_GAME_LOADING_SUCCESS:
	{
		Process_Game_Start(user_id);
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

	m_clients[user_id].set_user_position(packet->position);
	m_clients[user_id].set_user_velocity(packet->velocity);
	m_clients[user_id].set_user_yaw(packet->yaw);

	//for (auto ptr = m_clients[user_id].view_list.begin(); ptr != m_clients[user_id].view_list.end(); ++ptr)
	//	send_move_packet(*ptr, user_id, packet->position);

	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
		send_move_packet(*ptr, user_id, packet->position);
}

void cGameServer::Process_Chat(const int user_id, void* buff)
{
	cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(buff);
	char mess[MAX_CHAT_SIZE];

	strcpy_s(mess, packet->message);

	// ���� �濡 �ִ� �������׸� �޼��� ���� ����
	//m_clients[user_id]._room_list_lock.lock();
	//for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
	//	send_chat_packet(*ptr, user_id, mess);
	//m_clients[user_id]._room_list_lock.unlock();

	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	

	for (int i = 0; i < 6; ++i)
	{
		if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
			send_chat_packet(rl.Get_Join_Member(i), user_id, mess);
	}
}

void cGameServer::Process_Create_Room(const unsigned int _user_id) // ��û���� ���ο� �� ����
{
	m_clients[_user_id].set_join_room_number(m_room_manager->Create_room(_user_id));
	m_clients[_user_id].set_state(CLIENT_STATE::ST_GAMEROOM);

	send_create_room_ok_packet(_user_id, m_clients[_user_id].get_join_room_number());
}

void cGameServer::Process_Join_Room(const int user_id, void* buff)
{
	cs_packet_join_room* packet = reinterpret_cast<cs_packet_join_room*>(buff);
	
	
	if (m_room_manager->Get_Room_Info(packet->room_number)->_room_state == GAME_ROOM_STATE::READY)
	{
		if (m_room_manager->Join_room(user_id, packet->room_number))
		{
			m_clients[user_id].set_join_room_number(packet->room_number);
			m_clients[user_id].set_state(CLIENT_STATE::ST_GAMEROOM);
			Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id)
				{
					m_clients[user_id]._room_list_lock.lock();
					m_clients[user_id].room_list.insert(rl.Get_Join_Member(i));
					m_clients[user_id]._room_list_lock.unlock();
				}
			}


			// ���� ���⿡ �� �濡 �����ϴ� ��� ������� ���� �����ߴ��� ������ �����ؾ���!
			for (int i = 0; i < 6; ++i)
			{
				if (rl.Get_Join_Member(i) != -1 && rl.Get_Join_Member(i) != user_id) 
				{
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.lock();
					m_clients[rl.Get_Join_Member(i)].room_list.insert(user_id);
					m_clients[rl.Get_Join_Member(i)]._room_list_lock.unlock();
				}
			}

			send_join_room_success_packet(user_id);
		}
		else
			send_join_room_fail_packet(user_id);
	}
}

void cGameServer::Process_Exit_Room(const int user_id, void* buff)
{
	cs_packet_request_exit_room* packet = reinterpret_cast<cs_packet_request_exit_room*>(buff);
	
	if (m_clients[user_id].get_join_room_number() != -1) {
		Room& cl_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
		cl_room.Exit_Player(user_id);
		m_clients[user_id].set_join_room_number(-1);
		m_clients[user_id].set_state(CLIENT_STATE::ST_LOBBY);

		for (auto& p : m_clients[user_id].room_list)
		{
			m_clients[p]._room_list_lock.lock();
			m_clients[p].room_list.erase(m_clients[p].room_list.find(user_id));
			m_clients[p]._room_list_lock.unlock();
		}

		m_clients[user_id]._room_list_lock.lock();
		m_clients[user_id].room_list.clear();
		m_clients[user_id]._room_list_lock.unlock();

		// ���� ������ �κ��̹Ƿ� �� �������� �ٽ� ���������!
		sc_packet_request_room_info send_packet;
		int room_number;
		for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1�������� 10���� ���� ���δٰ� ����, �������� ������ �غ���
		{
			room_number = i + (packet->request_page * MAX_ROOM_INFO_SEND);
			Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
			send_packet.room_info[i].room_number = room_number;
			send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
			send_packet.room_info[i].state = get_room_info._room_state;
			get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 20);
		}

		if (sizeof(send_packet) >= CHECK_MAX_PACKET_SIZE)
		{
			send_packet.size = CHECK_MAX_PACKET_SIZE;
			send_packet.sub_size_mul = sizeof(sc_packet_request_room_info) / CHECK_MAX_PACKET_SIZE;
			send_packet.sub_size_add = sizeof(sc_packet_request_room_info) % CHECK_MAX_PACKET_SIZE;
		}
		else
			send_packet.size = sizeof(sc_packet_request_room_info);
		send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
		m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
	}
}

void cGameServer::Process_Ready(const int user_id, void* buff)
{
	cs_packet_ready* packet = reinterpret_cast<cs_packet_ready*>(buff);

	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	cout << "id : " << user_id << "�� ready��" << endl;
	cout << rl.All_Player_Ready() << endl;
	rl.SetReady(packet->ready_type, user_id);
	if (rl.All_Player_Ready())
	{
		for (auto p : rl.in_player) {
			send_game_start_packet(p);
			m_clients[p]._state_lock.lock();
			m_clients[p].set_state(CLIENT_STATE::ST_INGAME);
			m_clients[p]._state_lock.unlock();
		}
		// ��� �÷��̾ ���� �� ��� ���� ������ �����ϰ� �ٲ�����ϴ� �κ�!
		rl._room_state_lock.lock();
		rl._room_state_lock.unlock();
	}
}

void cGameServer::Process_Game_Start(const int user_id)
{
	Room& rl = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	rl.SetLoading(true, user_id);
	if (rl.All_Player_Loading())
		rl.Start_Game();
}

void cGameServer::Process_Request_Room_Info(const int user_id, void* buff)
{
	cs_packet_request_all_room_info* packet = reinterpret_cast<cs_packet_request_all_room_info*>(buff);

	sc_packet_request_room_info send_packet;

	int room_number;
	for (int i = 0; i < MAX_ROOM_INFO_SEND; ++i) // 1�������� 10���� ���� ���δٰ� ����, �������� ������ �غ���
	{
		room_number = i + (packet->request_page * MAX_ROOM_INFO_SEND);
		Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
		send_packet.room_info[i].room_number = room_number;
		send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
		send_packet.room_info[i].state = get_room_info._room_state;
		get_room_info.Get_Room_Name(send_packet.room_info[i].room_name, 20);
	}

	if (sizeof(sc_packet_request_room_info) >= 127)
	{
		send_packet.size = 127;
		send_packet.sub_size_mul = sizeof(sc_packet_request_room_info) / 127;
		send_packet.sub_size_add = sizeof(sc_packet_request_room_info) % 127;
	}
	else
		send_packet.size = sizeof(sc_packet_request_room_info);
	send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
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
		m_clients[c_id].set_name(packet->id);
		m_clients[c_id].set_login_state(Y_LOGIN);
		m_clients[c_id].set_state(CLIENT_STATE::ST_LOBBY);
	}
	else
	{
		if (reason == 0)
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::INVALID_ID);
		else
			send_login_fail_packet(c_id, LOGIN_FAIL_REASON::WRONG_PW);
	}
}

int cGameServer::get_new_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		m_clients[i]._state_lock.lock();
		if (CLIENT_STATE::ST_FREE == m_clients[i].get_state())
		{
			m_clients[i].set_state(CLIENT_STATE::ST_ACCEPT);
			m_clients[i]._state_lock.unlock();
			return i;
		}
		m_clients[i]._state_lock.unlock();
	}

	std::cout << "Maximum Number of Clients Overflow! \n";
	return -1;
}
