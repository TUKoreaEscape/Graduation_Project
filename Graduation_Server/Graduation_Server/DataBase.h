#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>

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

	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retcode);

	int check_login(std::wstring user_id, std::wstring user_pw);

	void show_error();

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szID[NAMELEN];
	SQLWCHAR szPW[NAMELEN];
	
	SQLLEN cbID = 0;
	SQLLEN cbPW = 0;
};