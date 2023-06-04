#include "GameServer.h"

cGameServer& cGameServer::GetInstance()
{
	static cGameServer instance;
	return instance;
}

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
	m_room_manager->init_object();

	m_session_timer.reset();
}

void cGameServer::StartServer()
{
	C_IOCP::Start_server();

	for (auto& player : m_clients)
	{
		player.set_user_position({ 0,5,0 });
	}
	// 이쪽은 이제 맵 로드할 부분
	for (int i = 0; i < 6; ++i)
		m_worker_threads.emplace_back(std::thread(&cGameServer::WorkerThread, this));

	for(int i = 0; i < 2; ++i)
		m_timer_thread.emplace_back(std::thread(&cGameServer::Update_Session,this, i));

	for (int i = 0; i < 1; ++i)
		m_database_thread.emplace_back(std::thread(&DataBase::DataBaseThread, m_database));

	for (auto& worker : m_worker_threads)
		worker.join();
	
	for (auto& timer_worker : m_timer_thread)
		timer_worker.join();

	for (auto& db_worker : m_database_thread)
		db_worker.join();
}

void cGameServer::WorkerThread()
{
	while (true)
	{
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over{};
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
			break;

		case OP_SEND:
			if (num_byte != exp_over->m_wsa_buf.len) {
				std::cout << "send_error" << std::endl;
				int WSAerror = WSAGetLastError();
				Disconnect(client_id);
				delete exp_over;
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
#if DEBUG
		std::cout << "Accept Completed \n";
#endif
	unsigned int new_id = get_new_id();
	if (-1 == new_id) {}
	else
	{
#if DEBUG
		cout << "Accept Client! new_id : " << new_id << endl;
#endif
		SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->m_buf));
		m_clients[new_id]._socket = c_socket;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), C_IOCP::m_h_iocp, new_id, 0);
		m_clients[new_id].do_recv();
		m_clients[new_id].set_id(new_id);
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

void cGameServer::Send(EXP_OVER* exp_over)
{

}

void cGameServer::Disconnect(const unsigned int _user_id) // 클라이언트 연결을 풀어줌(비정상 접속해제 or 정상종료시 사용)
{
	CLIENT& cl = m_clients[_user_id];

	string cl_name{};

	char client_name[20];
	cl.get_client_name(*client_name, sizeof(client_name));
	cl_name = client_name;

	if (!cl_name.empty()) {
		wstring convertID = stringToWstring(cl_name);
		Custom custom_data;
		custom_data.body = cl.m_customizing->Get_Body_Custom();
		custom_data.body_parts = cl.m_customizing->Get_Body_Part_Custom();
		custom_data.eyes = cl.m_customizing->Get_Eyes_Custom();
		custom_data.gloves = cl.m_customizing->Get_Gloves_Custom();
		custom_data.head = cl.m_customizing->Get_Head_Custom();
		custom_data.mouthandnoses = cl.m_customizing->Get_Mouthandnoses_Custom();

		DB_Request request;
		request.type = REQUEST_SAVE_CUSTOMIZING;
		request.request_custom_data = custom_data;
		request.request_id = _user_id;
		request.request_custom_data;
		request.request_name = convertID;
		m_database->insert_request(request);
	}
	// 여기서 초기화
	if (cl.get_join_room_number() != -1) {
		int disconnect_room_number = cl.get_join_room_number();
		Room& rl = *m_room_manager->Get_Room_Info(cl.get_join_room_number());
		for (int i = 0; i < 6; ++i)
		{
			if (rl.Get_Join_Member(i) != _user_id && rl.Get_Join_Member(i) != -1)
			{
				int rl_id = rl.Get_Join_Member(i);
				m_clients[rl_id]._room_list_lock.lock();
				m_clients[rl_id].room_list.erase(m_clients[rl_id].room_list.find(_user_id));
				m_clients[rl_id]._room_list_lock.unlock();
			}
		}
		rl.Exit_Player(_user_id);

		sc_packet_update_room update_room_packet;
		update_room_packet.size = sizeof(update_room_packet);
		update_room_packet.type = SC_PACKET::SC_PACKET_ROOM_INFO_UPDATE;
		update_room_packet.join_member = m_room_manager->Get_Room_Info(disconnect_room_number)->Get_Number_of_users();
		update_room_packet.room_number = disconnect_room_number;
		update_room_packet.state = m_room_manager->Get_Room_Info(disconnect_room_number)->_room_state;

		for (auto& cl : m_clients)
		{
			if (cl.get_id() == _user_id)
				continue;

			if (cl.get_state() != CLIENT_STATE::ST_LOBBY)
				continue;

			if (cl.get_look_lobby_page() != disconnect_room_number / 6)
				continue;

			cl.do_send(sizeof(update_room_packet), &update_room_packet);
		}
	}

	// 여긴 같은 방에 있는 플레이어에게 사용자가 떠나버림을 알려줌
	sc_other_player_disconnect packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET::SC_PACKET_OTHER_PLAYER_DISCONNECT;
	packet.id = _user_id;
	for (auto p : cl.room_list)
	{
		m_clients[p].do_send(sizeof(packet), &packet);
	}
	//==============================================================
	cl._room_list_lock.lock();
	cl.room_list.clear();
	cl._room_list_lock.unlock();
	cl.set_user_position({ 0,5,0 });
	cl.set_user_velocity({ 0,0,0 });
	cl.set_user_yaw(0);
	closesocket(cl._socket);

	cl._state_lock.lock();
	cl.set_state(CLIENT_STATE::ST_FREE);
	cl.set_login_state(N_LOGIN);
	char reset_name[20] = {};
	cl.set_name(reset_name);
	cl.set_id(-1);
	cl._state_lock.unlock();
}

wstring cGameServer::stringToWstring(const std::string& t_str) // string -> wstring 변환
{
	typedef codecvt_utf8<wchar_t>  convert_type;
	wstring_convert<convert_type, wchar_t> converter;

	return converter.from_bytes(t_str);

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

CollisionInfo cGameServer::GetCollisionInfo(const BoundingOrientedBox& other, const BoundingOrientedBox& moved)
{
	CollisionInfo collisionInfo;
	collisionInfo.collision_face_num = -1;
	float penetrationDepth = 0.0f;
	XMFLOAT3 collisionNormal(0.0f, 0.0f, 0.0f);
	XMFLOAT3 collisionPoint(0.0f, 0.0f, 0.0f);

	XMVECTOR OrientedWorldNormals[6];
	XMVECTOR normals[6];
	normals[0] = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // x-axis
	normals[1] = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // y-axis
	normals[2] = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // z-axis
	normals[3] = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f); // -x-axis
	normals[4] = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f); // -y-axis
	normals[5] = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f); // -z-axis

	OrientedWorldNormals[0] = normals[0];
	OrientedWorldNormals[1] = normals[1];
	OrientedWorldNormals[2] = normals[2];
	OrientedWorldNormals[3] = normals[3];
	OrientedWorldNormals[4] = normals[4];
	OrientedWorldNormals[5] = normals[5];
	for (int i = 0; i < 6; ++i)
	{
		XMFLOAT3 tp;
		XMStoreFloat3(&tp, OrientedWorldNormals[i]);
	}

	if (other.Intersects(moved))
	{
		UINT collidedFaceIndex = 0;
		float minPenetrationDepth = FLT_MAX;

		for (UINT i = 0; i < 6; i++)
		{
			XMFLOAT3 faceNormal;
			XMStoreFloat3(&faceNormal, OrientedWorldNormals[i]);
			float distanceToPlane = DistanceToPlane(other.Center, faceNormal, moved.Center);
			if (distanceToPlane > 0.0f)
			{
				float penetration = ((other.Extents.x * abs(faceNormal.x)) +
					(other.Extents.y * abs(faceNormal.y)) +
					(other.Extents.z * abs(faceNormal.z))) - distanceToPlane;

				if (penetration < minPenetrationDepth)
				{
					collidedFaceIndex = i;
					minPenetrationDepth = penetration;
					collisionInfo.collision_face_num = i;
				}
			}
		}
		XMFLOAT3 temp;
		temp.x = moved.Center.x - other.Center.x;
		temp.y = moved.Center.y - other.Center.y;
		temp.z = moved.Center.z - other.Center.z;

		XMStoreFloat3(&collisionNormal, OrientedWorldNormals[collidedFaceIndex]);
		if (Dot(collisionNormal, temp) < 0.0f)
		{
			collisionNormal.x *= -1;
			collisionNormal.y *= -1;
			collisionNormal.z *= -1;
		}
	}

	collisionInfo.CollisionNormal = collisionNormal;

	return collisionInfo;
}

CLIENT* cGameServer::get_client_info(const int player_id)
{
	return &m_clients[player_id];
}



//============================================================================
// 패킷 구분후 처리해주는 공간
//============================================================================
void cGameServer::ProcessPacket(const unsigned int user_id, unsigned char* p) // 패킷구분 후 처리
{
	unsigned char packet_type = p[1];

	switch (packet_type) // 패킷 타입별로 처리할 공간
	{
	case CS_PACKET::CS_PACKET_LOGIN:
	{
		// 로그인 처리
		Process_User_Login(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_CREATE_ID:
	{
		Process_Create_ID(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_MOVE:
	{
		if(m_clients[user_id].get_login_state() == Y_LOGIN && m_clients[user_id].get_join_room_number() != -1)
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
#if !DEBUG
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_LOBBY) // 로그인하고 로비에 있을때만 방 생성 가능
#endif
			Process_Create_Room(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_JOIN_ROOM:
	{
#if !DEBUG
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_LOBBY)
#endif
#if !DEBUG
			if (m_clients[user_id].get_state() == CLIENT_STATE::ST_LOBBY)
#endif
		cout << "recv Join Room" << endl;
		Process_Join_Room(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_READY:
	{
#if !DEBUG
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_GAMEROOM)
#endif
			Process_Ready(user_id, p);
		break;
	}
	
	case CS_PACKET::CS_PACKET_EXIT_ROOM:
	{
#if !DEBUG
		if (Y_LOGIN == m_clients[user_id].get_login_state() && m_clients[user_id].get_state() == CLIENT_STATE::ST_GAMEROOM)
#endif
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

	case CS_PACKET::CS_PACKET_ATTACK:
	{
		Process_Attack(user_id);
		break;
	}

	case CS_PACKET::CS_PACKET_REQUEST_OPEN_DOOR:
	{

		Process_Door(user_id, p);
		break;
	}

	case CS_PACKET::CS_PACKET_CUSTOMIZING: // 커스터마이징 데이터를 처리합니다.
	{
		Process_Customizing(user_id, p);
		break;
	}
	}
}



