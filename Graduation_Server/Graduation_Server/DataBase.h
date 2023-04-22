#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include "User_Custom.h"
#include <sqlext.h>
#include <queue>

constexpr auto NAMELEN = 40;

struct UserData_ID_PW {
	SQLWCHAR user_id[NAMELEN];
	SQLWCHAR user_pw[NAMELEN];
	//std::wstring user_id;
	//std::wstring user_pw;
};

enum REQUEST_TYPE {REQUEST_LOGIN = 0, REQUEST_CREATE_ID, REQUEST_LOAD_CUSTOMIZING, REQUEST_SAVE_CUSTOMIZING};

struct DB_Request {
	REQUEST_TYPE	type{};
	std::wstring	request_name{};
	char			request_char_name[20];
	std::wstring	request_pw{};
	int				request_id;
};

class DataBase
{
public:
	DataBase();
	~DataBase();

	void	HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retcode);

	int		check_login(std::wstring user_id, std::wstring user_pw);
	int		create_id(std::wstring user_id, std::wstring user_pw);

	Custom	Load_Customizing(std::wstring user_id);
	int		Save_Customizing(std::wstring user_id, Custom& save_data);

	void	show_error();

	void	insert_request(DB_Request& request);

	void	DataBaseThread();

private:
	std::queue<DB_Request>	request_db_queue;

	SQLHENV henv{};
	SQLHDBC hdbc{};
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode = 0;

	SQLWCHAR szID[NAMELEN]{};
	SQLWCHAR szPW[NAMELEN]{};
	
	SQLINTEGER custom_head = 0;
	SQLINTEGER custom_head_parts = 0;
	SQLINTEGER custom_body = 0;
	SQLINTEGER custom_body_parts = 0;
	SQLINTEGER custom_eyes = 0;
	SQLINTEGER custom_gloves = 0;
	SQLINTEGER custom_mouthandnose = 0;
	SQLINTEGER custom_tails = 0;

	SQLLEN cbID = 0;
	SQLLEN cbPW = 0;

	SQLLEN cb_custom_head = 0;
	SQLLEN cb_custom_head_parts = 0;
	SQLLEN cb_custom_body = 0;
	SQLLEN cb_custom_body_parts = 0;
	SQLLEN cb_custom_eyes = 0;
	SQLLEN cb_custom_gloves = 0;
	SQLLEN cb_custom_mouthandnose = 0;
	SQLLEN cb_custom_tails = 0;
};