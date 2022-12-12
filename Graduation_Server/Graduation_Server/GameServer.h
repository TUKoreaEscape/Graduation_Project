#pragma once
#include "stdafx.h"
#include "IOCP.h"
#include "EXPOver.h"
#include "Voice_chat.h"
#include "User_Client.h"

class EXP_OVER;
class CLIENT;

class cGameServer : public C_IOCP
{
public:
	cGameServer();
	~cGameServer();

	void	init();
	void	StartServer();
	void	WorkerThread();
	void	ProcessPacket(const unsigned int user_id, unsigned char* p);

	void	Accept(EXP_OVER* exp_over);
	void	Send(EXP_OVER* exp_over);
	void	Recv(EXP_OVER* exp_over, const unsigned int user_id, const DWORD num_byte);
	void	Disconnect(const unsigned int _user_id);

	int		get_new_id();

private:
	std::vector<std::thread>		m_worker_threads;
	std::thread						m_timer_thread;

	std::array<CLIENT, MAX_USER>	m_clients;
	//static Voice_Chat					*m_voice_chat;
	
};