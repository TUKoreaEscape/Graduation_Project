#pragma once
#include <codecvt>
#include "stdafx.h"
#include "IOCP.h"
#include "EXPOver.h"
#include "Voice_chat.h"
#include "User_Client.h"
#include "Room_Manager.h"
#include "DataBase.h"
#include "Server_Timer.h"

class EXP_OVER;
class CLIENT;
class Voice_Chat;
class RoomManager;

class cGameServer : public C_IOCP
{
public:
	cGameServer();
	~cGameServer();

	static cGameServer& GetInstance();

	void	init();
	void	StartServer();
	void	WorkerThread();
	



	void	Accept(EXP_OVER* exp_over);
	void	Send(EXP_OVER* exp_over);
	void	Recv(EXP_OVER* exp_over, const unsigned int user_id, const DWORD num_byte);
	void	Disconnect(const unsigned int _user_id);
	void	Update_Session(); // 방의 시간, 실시간으로 체크해야하는 부분은 여기서 전부 처리 할 예정입니다.

	void	send_chat_packet(const unsigned int user_id, const unsigned int my_id, char* mess);
	void	send_login_fail_packet(const unsigned int user_id, LOGIN_FAIL_REASON::TYPE reason);
	void	send_login_ok_packet(const unsigned int user_id);
	void	send_create_id_ok_packet(const unsigned int user_id);
	void	send_create_id_fail_packet(const unsigned int user_id, char reason);
	void	send_create_room_ok_packet(const unsigned int user_id, const int room_number);
	void	send_join_room_success_packet(const unsigned int user_id);
	void	send_join_room_fail_packet(const unsigned int user_id);
	void	send_move_packet(const unsigned int id, const unsigned int moved_id, XMFLOAT3 pos);
	void	send_game_start_packet(const unsigned int id);

	void	send_voice_data(const unsigned int id);



	void	ProcessPacket(const unsigned int user_id, unsigned char* p); // 패킷을 구분후 처리함
	void	Process_User_Login(int c_id, void* buff);
	void	Process_Create_ID(int c_id, void* buff);
	void	Process_Create_Room(const unsigned int _user_id);
	void	Process_Move(const int user_id, void* buff);
	void	Process_Chat(const int user_id, void* buff);
	void	Process_Request_Room_Info(const int user_id, void* buff);
	void	Process_Join_Room(const int user_id, void* buff);
	void	Process_Exit_Room(const int user_id, void* buff);
	void	Process_Ready(const int user_id, void* buff);
	void	Process_Game_Start(const int user_id);

	// voice chat data를 전송하는 부분!
	void	Process_Voice_Data(int user_id);

	int		get_new_id();

	wstring stringToWstring(const std::string& t_str);

private:
	std::vector<std::thread>		m_worker_threads;
	std::vector<std::thread>		m_timer_thread;
	
	std::array<CLIENT, MAX_USER>	m_clients;
	Voice_Chat						*m_voice_chat = nullptr;
	RoomManager						*m_room_manager = nullptr;
	DataBase						*m_database = nullptr;
	Server_Timer					m_PerformanceCounter;
	Server_Timer					m_session_timer;
};