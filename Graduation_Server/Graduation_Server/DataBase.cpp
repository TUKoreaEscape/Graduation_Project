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
				//std::cout << "Login Success!! ID : " << debug_id << "\n";
				SQLCancel(hstmt);
				delete check_data;
				return 1;
			}

			std::string debug_id;
			debug_id.assign(user_id.begin(), user_id.end());
			//std::cout << "Login Fail!! ID : " << debug_id << "\n";
			SQLCancel(hstmt);
			delete check_data;
			return 2;
		}
		//std::cout << "ID doesn't exist. \n";
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

	//std::cout << "ID�� ���°��� Ȯ��! ������ �õ��մϴ�." << std::endl;
	std::wstring insert_query{};
	Custom init_data;
	init_data.body = MAINBODY01;
	init_data.body_parts = BODYPART01;
	init_data.eyes = EYE01;
	init_data.gloves = GLOVE01;
	init_data.head = HEAD01;
	init_data.mouthandnoses = MOUTH01;


	insert_query += L"EXEC Insert_id ";
	insert_query += user_id;
	insert_query += L", ";
	insert_query += user_pw;
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.head);
	insert_query += L", ";
	insert_query += std::to_wstring(0);
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.body);
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.body_parts);
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.eyes);
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.gloves);
	insert_query += L", ";
	insert_query += std::to_wstring(init_data.mouthandnoses);
	insert_query += L", ";
	insert_query += std::to_wstring(0);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)insert_query.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // ���⸦ ����� ��� id�� DB���� ���� �����!
	{
		return 1; // �����Ϸ�
	}

	return 2; // �׳� ������
}

Custom DataBase::Load_Customizing(std::wstring user_id)
{
	std::wstring wp{};

	Custom custom_data = {};
	wp += L"EXEC Get_Custom ";
	wp += user_id;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wp.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &custom_head, 10, &cb_custom_head);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &custom_head_parts, 10, &cb_custom_head_parts);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &custom_body, 10, &cb_custom_body);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &custom_body_parts, 10, &cb_custom_body_parts);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &custom_eyes, 10, &cb_custom_eyes);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &custom_gloves, 10, &cb_custom_gloves);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &custom_mouthandnose, 10, &cb_custom_mouthandnose);
		retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &custom_tails, 10, &cb_custom_tails);

		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			custom_data.head = static_cast<HEADS>(custom_head);
			custom_data.body = static_cast<BODIES>(custom_body);
			custom_data.body_parts = static_cast<BODYPARTS>(custom_head_parts);
			custom_data.eyes = static_cast<EYES>(custom_eyes);
			custom_data.gloves = static_cast<GLOVES>(custom_gloves);
			custom_data.mouthandnoses = static_cast<MOUTHANDNOSES>(custom_mouthandnose);
		}
	}

	return custom_data;
}

int DataBase::Save_Customizing(std::wstring user_id, Custom& save_data)
{
	std::wstring wp{};

	wp += L"EXEC Save_Custom ";
	wp += user_id;
	wp += L", ";
	wp += std::to_wstring(save_data.head);
	wp += L", ";
	wp += std::to_wstring(0);
	wp += L", ";
	wp += std::to_wstring(save_data.body);
	wp += L", ";
	wp += std::to_wstring(save_data.body_parts);
	wp += L", ";
	wp += std::to_wstring(save_data.eyes);
	wp += L", ";
	wp += std::to_wstring(save_data.gloves);
	wp += L", ";
	wp += std::to_wstring(save_data.mouthandnoses);
	wp += L", ";
	wp += std::to_wstring(0);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wp.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		return 0;
	return 1;
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