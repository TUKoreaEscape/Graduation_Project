#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include <sqlext.h>

constexpr auto NAMELEN = 11;

struct UserData_ID_PW {
	std::wstring user_id;
	std::wstring user_pw;
};

class DataBase
{
public:
	DataBase();
	~DataBase();

	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retcode);

	bool check_login(std::wstring user_id, std::wstring user_pw);

	void show_error();

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szID[NAMELEN];
	SQLWCHAR szPW[NAMELEN + 10];

	SQLLEN cbID = 0;
	SQLLEN cbPW = 0;
};