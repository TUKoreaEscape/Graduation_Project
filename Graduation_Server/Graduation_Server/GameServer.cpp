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
	// 서버 시작시 기본 초기화는 여기서 해줄 예정!
	m_database = new DataBase;
	
	// 초기화함수 곧 만들거임! 아마도 ㅎ

	m_voice_chat = new Voice_Chat;
	m_voice_chat->Init();

	m_room_manager = new RoomManager;
	m_room_manager->init();
}

void cGameServer::StartServer()
{
	C_IOCP::Start_server();

	// 이쪽은 이제 맵 로드할 부분
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
			cout << client_id << "_send \n";
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

wstring cGameServer::stringToWstring(const std::string& t_str) // string -> wstring 변환
{
	typedef codecvt_utf8<wchar_t>  convert_type;
	wstring_convert<convert_type, wchar_t> converter;

	return converter.from_bytes(t_str);

}

void cGameServer::Send(EXP_OVER* exp_over)
{

}

void cGameServer::Disconnect(const unsigned int _user_id) // 클라이언트 연결을 풀어줌(비정상 접속해제 or 정상종료시 사용)
{
	CLIENT& cl = m_clients[_user_id];
	
	Room& rl = *m_room_manager->Get_Room_Info(cl.get_join_room_number());
	// 여기서 초기화
	if (cl.get_join_room_number() != -1) {
		cout << "있던방을 나감" << endl;
		rl.Exit_Player(_user_id);
		cout << "m_room_manager->Get_Room_Info(cl.get_join_room_number())의 실행값 : " << rl.Get_Number_of_users() << endl;
	}
	cl._state_lock.lock();
	cl.set_state(ST_FREE);
	cl.set_login_state(N_LOGIN);
	cl.set_id(-1);
	cl._state_lock.unlock();
	closesocket(cl._socket);
}


//============================================================================
// 패킷 구분후 처리해주는 공간
//============================================================================
void cGameServer::ProcessPacket(const unsigned int user_id, unsigned char* p) // 패킷구분 후 처리
{
	unsigned char packet_type = p[1];

	switch (packet_type) // 패킷 타입별로 처리할 공간
	{
	case CS_PACKET::CS_LOGIN:
	{
		// 로그인 처리
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
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == ST_LOBBY) // 로그인하고 로비에 있을때만 방 생성 가능
			Process_Create_Room(user_id);
		break;
	}

	case CS_PACKET::CS_PACKET_JOIN_ROOM:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == ST_LOBBY)
			Process_Join_Room(user_id, p);
		break;
	}
	
	case CS_PACKET::CS_PACKET_EXIT_ROOM:
	{
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == ST_LOBBY)
			Process_Exit_Room(user_id, p);
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
// 구분한 패킷 처리하는 공간
//============================================================================
void cGameServer::Process_Create_ID(int c_id, void* buff) // 요청받은 ID생성패킷 처리
{
	int reason = 0;

	cs_packet_create_id* packet = reinterpret_cast<cs_packet_create_id*>(buff);

	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->create_id(stringToWstring(stringID), stringToWstring(stringPW));

	cout << "reason : " << reason << endl;
	if (reason == 1) // id 생성 성공
		send_create_id_ok_packet(c_id);
	else // id 생성 실패
		send_create_id_fail_packet(c_id, reason);
}

void cGameServer::Process_Move(const int user_id, void* buff) // 요청받은 캐릭터 이동을 처리
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

	// 같은 방에 있는 유저한테만 메세지 보낼 예정
	m_clients[user_id]._room_list_lock.lock();
	for (auto ptr = m_clients[user_id].room_list.begin(); ptr != m_clients[user_id].room_list.end(); ++ptr)
		send_chat_packet(*ptr, user_id, mess);
	m_clients[user_id]._room_list_lock.unlock();
}

void cGameServer::Process_Create_Room(const unsigned int _user_id) // 요청받은 새로운 방 생성
{
	sc_packet_create_room packet;

	m_clients[_user_id].set_join_room_number(m_room_manager->Create_room(_user_id));
	m_clients[_user_id].set_state(ST_GAMEROOM);

	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_CREATE_ROOM_OK;
	packet.room_number = m_clients[_user_id].get_join_room_number();

	m_clients[_user_id].do_send(sizeof(packet), &packet);
}

void cGameServer::Process_Join_Room(const int user_id, void* buff)
{
	cs_packet_join_room* packet = reinterpret_cast<cs_packet_join_room*>(buff);
	
	m_clients[user_id].set_join_room_number(packet->room_number);
	m_clients[user_id].set_state(ST_GAMEROOM);

	m_room_manager->Join_room(user_id, packet->room_number);
	// 여기서 방 입장 성공,실패 유무를 판단후 클라에게 재전송
}

void cGameServer::Process_Exit_Room(const int user_id, void* buff)
{
	cs_packet_request_exit_room* packet = reinterpret_cast<cs_packet_request_exit_room*>(buff);
	m_clients[user_id].set_state(ST_LOBBY);

	// 방을 나가면 로비이므로 방 정보들을 다시 보내줘야함!
	sc_packet_request_room_info send_packet;


	int room_number;
	for (int i = 0; i < 10; ++i)
	{
		room_number = i + (packet->request_page * 10);
		Room &get_room_info = *m_room_manager->Get_Room_Info(room_number);
		send_packet.room_info[i].room_number = room_number;
		send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
		send_packet.room_info[i].state = get_room_info._room_state;
	}
	send_packet.size = sizeof(sc_packet_request_room_info);
	send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
	m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
}

void cGameServer::Process_Request_Room_Info(const int user_id, void* buff)
{
	cs_packet_request_all_room_info* packet = reinterpret_cast<cs_packet_request_all_room_info*>(buff);

	sc_packet_request_room_info send_packet;


	
	int room_number;
	for (int i = 0; i < 10; ++i) // 1페이지당 10개의 방이 보인다고 가정, 룸정보를 전송할 준비함
	{
		room_number = i + (packet->request_page * 10);
		Room& get_room_info = *m_room_manager->Get_Room_Info(room_number);
		cout << room_number << " : [" << get_room_info.Get_Number_of_users() << "/6]" << endl;
		send_packet.room_info[i].room_number = room_number;
		send_packet.room_info[i].join_member = get_room_info.Get_Number_of_users();
		send_packet.room_info[i].state = get_room_info._room_state;
	}
	send_packet.size = sizeof(sc_packet_request_room_info);
	send_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO;
	cout << "send_packet size : " << int(send_packet.size) << endl;
	m_clients[user_id].do_send(sizeof(send_packet), &send_packet);
}

void cGameServer::Process_User_Login(int c_id, void* buff) // 로그인 요청
{
	int reason = 0;

	cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buff);
	string stringID{};
	string stringPW{};

	stringID = packet->id;
	stringPW = packet->pass_word;
	reason = m_database->check_login(stringToWstring(stringID), stringToWstring(stringPW));
	if (reason == 1) // reason 0 : id가 존재하지 않음 / reason 1 : 성공 / reason 2 : pw가 틀림
	{
		send_login_ok_packet(c_id);
		m_clients[c_id].set_login_state(Y_LOGIN);
		m_clients[c_id].set_state(ST_LOBBY);
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
// 서버에서 보내는 패킷 함수들 
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
