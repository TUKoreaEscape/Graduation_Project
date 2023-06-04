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
#include <queue>

#define FIRST_SKILL_ENABLE_TIME 180
#define SECOND_SKILL_ENABLE_TIME 360
#define THIRD_SKILL_ENABLE_TIME 540
#define SET_SERVER_UPDATE_FRAME 30

class EXP_OVER;
class CLIENT;
class Voice_Chat;
class RoomManager;

enum class EventType : char
{
	DOOR_TIME
};

struct TIMER_EVENT {
	std::chrono::system_clock::time_point	event_time;
	EventType								event_type;
	float									cool_time;
	int										obj_id;

	constexpr bool operator < (const TIMER_EVENT& left) const
	{
		return (event_time > left.event_time);
	}
};

class cGameServer : public C_IOCP
{
public:
	static cGameServer* server_instance;
	cGameServer();
	~cGameServer();

public:

	//static cGameServer& GetInstance();
	static cGameServer* GetInstance() {
		if (server_instance == nullptr)
			server_instance = new cGameServer;
		return server_instance;
	}

	void	init();
	void	StartServer();
	void	WorkerThread();

	void	Accept(EXP_OVER* exp_over);
	void	Send(EXP_OVER* exp_over);
	void	Recv(EXP_OVER* exp_over, const unsigned int user_id, const DWORD num_byte);
	void	Disconnect(const unsigned int _user_id);
	void	Update_Session(int thread_number); // 방의 시간, 실시간으로 체크해야하는 부분은 여기서 전부 처리 할 예정입니다.

	void	Update_OtherPlayer(int room_number, float elaspeTime);
	void	Update_Viewlist(const int id, const int room_number);

	bool	Is_near(int a, int b);

	void	send_chat_packet(const unsigned int user_id, const unsigned int my_id, char* mess);
	void	send_login_fail_packet(const unsigned int user_id, LOGIN_FAIL_REASON::TYPE reason);
	void	send_login_ok_packet(const unsigned int user_id);
	void	send_create_id_ok_packet(const unsigned int user_id);
	void	send_create_id_fail_packet(const unsigned int user_id, char reason);
	void	send_create_room_ok_packet(const unsigned int user_id, const int room_number);
	void	send_join_room_success_packet(const unsigned int user_id);
	void	send_join_room_fail_packet(const unsigned int user_id);

	void	send_move_packet(const unsigned int id, const unsigned int moved_id, cs_packet_move recv_packet, XMFLOAT3 calculate_pos); // 한 클라이언트의 이동을 계산하여 다른 클라이언트에게 전송합니다.
	void	send_move_packet(const unsigned int id, const unsigned int moved_id); // 현재는 사용하지 않습니다/
	void	send_calculate_move_packet(const unsigned int id); // 이동을 요청한 클라이언트에게 좌표를 계산하여 넘겨줍니다.

	void	send_game_start_packet(const unsigned int id);
	void	send_put_player_data(const unsigned int recv_id);
	void	send_put_other_player(const unsigned int put_id, const unsigned int recv_id);
	void	send_voice_data(const unsigned int id);
	void	send_customizing_data(const unsigned int id);



	void	ProcessPacket(const unsigned int user_id, unsigned char* p); // 패킷을 구분후 처리함
	void	Process_User_Login(int c_id, void* buff);
	void	Process_Create_ID(int c_id, void* buff);
	void	Process_Create_Room(const unsigned int _user_id, void* buff);
	void	Process_Move(const int user_id, void* buff);
	void	Process_Chat(const int user_id, void* buff);
	void	Process_Request_Room_Info(const int user_id, void* buff);
	void	Process_Join_Room(const int user_id, void* buff);
	void	Process_Exit_Room(const int user_id, void* buff);
	void	Process_Ready(const int user_id, void* buff);
	void	Process_Game_Start(const int user_id);
	void	Process_Attack(const int user_id);
	void	Process_Customizing(const int user_id, void* buff);
	void	Process_Door(const int user_id, void* buff);
	
	void	Process_Event(const TIMER_EVENT& ev);

	// voice chat data를 전송하는 부분!
	void	Process_Voice_Data(int user_id);

	void	Timer();
	int		get_new_id();
	CLIENT*	get_client_info(const int player_id);
	CollisionInfo GetCollisionInfo(const BoundingOrientedBox& other, const BoundingOrientedBox& moved);

	wstring stringToWstring(const std::string& t_str);

	std::array<CLIENT, MAX_USER>	m_clients;
	DataBase						*m_database = nullptr;
protected:
	std::queue<CLIENT>				request_querry;

private:
	const int						RANGE = 7;

	std::vector<std::thread>		m_worker_threads;
	std::vector<std::thread>		m_timer_thread;
	std::vector<std::thread>		m_database_thread; // worker thread에서 쿼리 날리는건 손해다.

	Voice_Chat						*m_voice_chat = nullptr;
	RoomManager						*m_room_manager = nullptr;
	Server_Timer					m_PerformanceCounter;
	Server_Timer					m_session_timer;
	concurrency::concurrent_priority_queue<TIMER_EVENT> m_timer_queue;
};

