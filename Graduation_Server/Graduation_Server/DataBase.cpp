#include "DataBase.h"
#include <string>

DataBase::DataBase()
{
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"TaggerDB", SQL_NTS, (SQLWCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);

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

bool DataBase::check_login(std::wstring user_id, std::wstring user_pw)
{
	std::wstring wp{};

	wp += L"EXEC GetID ";
	wp += user_id;
	UserData_ID_PW *check_data = new UserData_ID_PW;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wp.c_str(), SQL_NTS);
	std::wcout << wp << std::endl;
	
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		// ID에 따른 저장된 PW를 불러옴
		retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, &check_data->user_id, NAMELEN, &cbID);
		retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, &check_data->user_pw, NAMELEN + 10, &cbPW);

		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			// PW비교후 아닌경우 FAIL 패킷 보내야함
			if (user_pw == check_data->user_pw)
			{
				SQLCancel(hstmt);
				wprintf(L"Login Success \n");
				delete check_data;
				return true;
			}
			SQLCancel(hstmt);
			wprintf(L"Login Fail \n");
			delete check_data;
			return false;
		}
	}   
	delete check_data;
	return false;
}

void DataBase::show_error()
{
	std::cout << "Error!!! \n";
}