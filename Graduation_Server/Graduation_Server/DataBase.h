#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include <sqlext.h>

constexpr auto NAMELEN = 11;

class DataBase
{
public:
	DataBase();
	~DataBase();

	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retcode);
	void show_error();

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szID[NAMELEN];
	SQLWCHAR szPW[NAMELEN + 10];
	SQLLEN cbID = 0;
};