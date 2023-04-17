#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include "User_Custom.h"
#include <sqlext.h>

constexpr auto NAMELEN = 40;

struct UserData_ID_PW {
	SQLWCHAR user_id[NAMELEN];
	SQLWCHAR user_pw[NAMELEN];
	//std::wstring user_id;
	//std::wstring user_pw;
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

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szID[NAMELEN];
	SQLWCHAR szPW[NAMELEN];
	
	SQLINTEGER custom_head;
	SQLINTEGER custom_head_parts;
	SQLINTEGER custom_body;
	SQLINTEGER custom_body_parts;
	SQLINTEGER custom_eyes;
	SQLINTEGER custom_gloves;
	SQLINTEGER custom_mouthandnose;
	SQLINTEGER custom_tails;

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