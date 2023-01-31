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

bool DataBase::check_login(std::wstring user_id, std::wstring user_pw)
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
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR , &check_data->user_pw, NAMELEN + 10, &cbPW);

		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		std::wcout << "Load id : " << user_id << std::endl;
		std::wcout << "Load pw : " << check_data->user_pw << std::endl;

		std::cout << sizeof(check_data->user_pw) << std::endl;
		std::cout << user_pw.size() << std::endl;
	

	
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			// PW���� �ƴѰ�� FAIL ��Ŷ ��������
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