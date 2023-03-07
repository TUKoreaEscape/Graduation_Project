#include "DataBase.h"
#include <string>

DataBase::DataBase()
{
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"Graduation_ODBC", SQL_NTS, (SQLWCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		std::cout << "DB Connect!" << std::endl;
}

DataBase::~DataBase()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv); 
}

int DataBase::check_login(std::wstring user_id, std::wstring user_pw)
{
	std::wstring wp{};

	wp += L"EXEC Get_PW ";
	wp += user_id;
	UserData_ID_PW *check_data = new UserData_ID_PW;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wp.c_str(), SQL_NTS);

	
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		// ID�� ���� ����� PW�� �ҷ���
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR , &check_data->user_pw, NAMELEN, &cbPW);

		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		std::wstring temp_pw(check_data->user_pw);
		temp_pw.erase(remove(temp_pw.begin(), temp_pw.end(), ' '), temp_pw.end());

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			// PW���� �ƴѰ�� FAIL ��Ŷ ��������
			if (user_pw.compare(temp_pw) == 0)
			{
				std::string debug_id;
				debug_id.assign(user_id.begin(), user_id.end());
				std::cout << "Login Success!! ID : " << debug_id << "\n";
				SQLCancel(hstmt);
				delete check_data;
				return 1;
			}

			std::string debug_id;
			debug_id.assign(user_id.begin(), user_id.end());
			std::cout << "Login Fail!! ID : " << debug_id << "\n";
			SQLCancel(hstmt);
			delete check_data;
			return 2;
		}
		std::cout << "ID doesn't exist. \n";
	}   
	delete check_data;
	return 0;
}

int DataBase::create_id(std::wstring user_id, std::wstring user_pw)
{

	std::wstring wp{};

	wp += L"EXEC Get_PW ";
	wp += user_id;
	UserData_ID_PW* check_data = new UserData_ID_PW;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wp.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // ���⸦ ����� ��� id�� DB���� ���� �����!
	{
		// ID�� ���� ����� PW�� �ҷ���
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &check_data->user_pw, NAMELEN, &cbPW);
		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		std::wstring temp_pw(check_data->user_pw);
		temp_pw.erase(remove(temp_pw.begin(), temp_pw.end(), ' '), temp_pw.end());

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLCancel(hstmt);
			return 0; // ID�� �ߺ���
		}
		SQLCancel(hstmt);
	}


	std::wstring insert_query{};
	insert_query += L"EXEC Insert_id ";
	insert_query += user_id;
	insert_query += L", ";
	insert_query += user_pw;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)insert_query.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // ���⸦ ����� ��� id�� DB���� ���� �����!
	{
		return 1; // �����Ϸ�
	}

	return 2; // �׳� ������
}

void DataBase::show_error()
{
	std::cout << "Error!!! \n";
}

void DataBase::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retcode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];

	if (retcode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid Handle");
		return;
	}

	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d) \n", wszState, wszMessage, iError);
		}
	}
}