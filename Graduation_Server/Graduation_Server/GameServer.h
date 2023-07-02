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
#define SECOND_SKILL_ENABLE_TIME 240
#define THIRD_SKILL_ENABLE_TIME 240
#define SET_SERVER_UPDATE_FRAME 30

class EXP_OVER;
class CLIENT;
class Voice_Chat;
class RoomManager;

enum class EventType : char
{
	CHECK_NUM_OF_SERVER_ACCEPT_USER,
	OPEN_DOOR,
	CLOSE_DOOR,
	OPEN_ELECTRONIC,
	CLOSE_ELECTRONIC,
	UPDATE_MOVE,
	PLAYER_ATTACK,
	PLAYER_VICTIM,
	SELECT_TAGGER,
	OPEN_TAGGER_SKILL_FIRST,
	OPEN_TAGGER_SKILL_SECOND,
	OPEN_TAGGER_SKILL_THIRD,
	GAME_END,
	SERVER_END
};

struct TIMER_EVENT {
	std::chrono::system_clock::time_point	event_time;
	EventType								event_type;
	float									cool_time;
	int										room_number;
	int										obj_id;

	constexpr bool operator < (const TIMER_EVENT& left) const
	{
		return (event_time > left.event_time);
	}
};

class cGameServer : public C_IOCP
{
private:
	static cGameServer* server_instance;
	cGameServer();
	~cGameServer();

public:
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
	void	Update_OtherPlayer(int room_number, float elaspeTime);
	void	Update_Viewlist(const int id, const int room_number);
	bool	Is_near(int a, int b);

public: // 이쪽은 패킷을 전송하는 함수의 모임입니다.
	void	send_chat_packet(const unsigned int user_id, const unsigned int my_id, char* mess); // 채팅내용을 패킷으로 보내는 함수입니다.
	void	send_login_fail_packet(const unsigned int user_id, LOGIN_FAIL_REASON::TYPE reason); // 로그인 실패를 패킷으로 전송하는 함수입니다.
	void	send_login_ok_packet(const unsigned int user_id); // 로그인에 성공했다는것을 패킷으로 전송하는 함수입니다.
	void	send_create_id_ok_packet(const unsigned int user_id); // 신규 아이디 생성에 성공했다는것을 패킷으로 전송하는 함수입니다.
	void	send_create_id_fail_packet(const unsigned int user_id, char reason); // 신규 아이디 생성에 실패했다는것을 패킷으로 전송하는 함수입니다.
	void	send_create_room_ok_packet(const unsigned int user_id, const int room_number); // 게임방에 생성을 성공했다는것을 알리는 함수입니다.
	void	send_join_room_success_packet(const unsigned int user_id);	// 방 접속에 성공했음을 알리는 패킷입니다.
	void	send_join_room_fail_packet(const unsigned int user_id);	// 방 접속에 실패했음을 알리는 패킷입니다.
	void	send_move_packet(const unsigned int id, const unsigned int moved_id); // 이동을 처리합니다.
	void	send_calculate_move_packet(const unsigned int id); // 이동을 요청한 클라이언트에게 좌표를 계산하여 넘겨줍니다.
	void	send_life_chip_update(const unsigned int id);
	void	send_game_start_packet(const unsigned int id); // 게임이 시작된것을 패킷으로 전송함
	void	send_put_player_data(const unsigned int recv_id);
	void	send_put_other_player(const unsigned int put_id, const unsigned int recv_id);  
	void	send_voice_data(const unsigned int id);
	void	send_customizing_data(const unsigned int id);


public:
	void	ProcessPacket(const unsigned int user_id, unsigned char* p); // 패킷을 구분후 처리함
	void	Process_User_Login(int c_id, void* buff); // 로그인 처리
	void	Process_Create_ID(int c_id, void* buff); // 아이디 생성 처리
	void	Process_Create_Room(const unsigned int _user_id, void* buff); // 방 생성을 처리하는 함수
	void	Process_Move(const int user_id, void* buff); // 이동을 처리하는 함수
	void	Process_Chat(const int user_id, void* buff); // 채팅을 처리하는 함수
	void	Process_Request_Room_Info(const int user_id, void* buff); // 방 정보를 요청하는 함수
	void	Process_Join_Room(const int user_id, void* buff); // 방 접속 요청을 처리하는 함수
	void	Process_Exit_Room(const int user_id, void* buff); // 방에서 나가는 요청을 처리하는 함수
	void	Process_Ready(const int user_id, void* buff); // 게임방에서 준비완료됨을 처리하는 함수
	void	Process_Game_Start(const int user_id); // 게임방에서 게임이 시작함을 처리하는 함수
	void	Process_Attack(const int user_id); // 공격을 처리하는 함수
	void	Process_Customizing(const int user_id, void* buff); // 커스터마이징 정보를 처리하는 함수
	void	Process_Door(const int user_id, void* buff); // 게임방 내 Door 열기,닫기를 처리하는 함수
	void	Process_Active_Altar(const int user_id);
	void	Process_Altar_LifeChip_Update(const int user_id);
	void	Process_ElectronicSystem_Open(const int user_id, void* buff); // 게임방 내 전력장치를 열기,닫기를 하는 함수
	void	Process_ElectronicSystem_Reset_By_Tagger(const int user_id, void* buff);
	void	Process_ElectronicSystem_Reset_By_Player(const int user_id, void* buff);
	void	Process_ElectronicSystem_Control(const int user_id, void* buff); // 게임방 내 전력장치 스위치 on,off를 처리, 수리를 체크하는 함수
	void	Process_ElectronicSystem_Activate(const int user_id, void* buff);
	void	Process_Pick_Fix_Item(const int user_id, void* buff);
	void	Process_Event(const TIMER_EVENT& ev); // 이벤트를 처리, worker thread로 넘기는 역할을 함 *매우중요*
	// voice chat data를 전송하는 부분!
	void	Process_Voice_Data(int user_id);
	// stress test용
	void	Process_Move_Test(const int user_id, void* buff);


public:
	bool	server_end = false;
	void	Server_End();
public:
	void	Timer();
	int		get_new_id();
	CLIENT*	get_client_info(const int player_id);
	CollisionInfo GetCollisionInfo(const BoundingOrientedBox& other, const BoundingOrientedBox& moved);

	wstring stringToWstring(const std::string& t_str);

public: // 이 아래 두개는 db와 플레이어용입니다.
	std::array<CLIENT, MAX_USER>	m_clients;
	DataBase						*m_database = nullptr;
protected:
	std::queue<CLIENT>				request_querry;

public: // 타이머 이벤트용

	concurrency::concurrent_priority_queue<TIMER_EVENT> m_timer_queue;
	std::priority_queue<TIMER_EVENT>					timer_queue;

private:
	const int						RANGE = 7;
	atomic<int>						join_member = 0;
	std::vector<std::thread>		m_worker_threads;
	std::vector<std::thread>		m_timer_thread;
	std::vector<std::thread>		m_database_thread; // worker thread에서 쿼리 날리는건 손해다.
	std::vector<std::thread>		m_event_thread;

	Voice_Chat						*m_voice_chat = nullptr;
	RoomManager						*m_room_manager = nullptr;
	Server_Timer					m_PerformanceCounter;
	Server_Timer					m_session_timer;
};

