#include "GameServer.h"
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
		// ID에 따른 저장된 PW를 불러옴
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR , &check_data->user_pw, NAMELEN, &cbPW);

		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		std::wstring temp_pw(check_data->user_pw);
		temp_pw.erase(remove(temp_pw.begin(), temp_pw.end(), ' '), temp_pw.end());

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			// PW비교후 아닌경우 FAIL 패킷 보내야함
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

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // 여기를 통과한 경우 id가 DB에는 없는 경우임!
	{
		// ID에 따른 저장된 PW를 불러옴
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &check_data->user_pw, NAMELEN, &cbPW);
		retcode = SQLFetch(hstmt);

		if (retcode == SQL_ERROR)
			show_error();

		std::wstring temp_pw(check_data->user_pw);
		temp_pw.erase(remove(temp_pw.begin(), temp_pw.end(), ' '), temp_pw.end());

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLCancel(hstmt);
			return 0; // ID가 중복임
		}
		SQLCancel(hstmt);
	}

	//std::cout << "ID가 없는것을 확인! 삽입을 시도합니다." << std::endl;
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
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) // 여기를 통과한 경우 id가 DB에는 없는 경우임!
	{
		return 1; // 생성완료
	}

	return 2; // 그냥 에러임
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
			custom_data.body_parts = static_cast<BODYPARTS>(custom_body_parts);
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
	wp += std::to_wstring(static_cast<int>(save_data.head));
	wp += L", ";
	wp += std::to_wstring(0);
	wp += L", ";
	wp += std::to_wstring(static_cast<int>(save_data.body));
	wp += L", ";
	wp += std::to_wstring(static_cast<int>(save_data.body_parts));
	wp += L", ";
	wp += std::to_wstring(static_cast<int>(save_data.eyes));
	wp += L", ";
	wp += std::to_wstring(static_cast<int>(save_data.gloves));
	wp += L", ";
	wp += std::to_wstring(static_cast<int>(save_data.mouthandnoses));
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

void DataBase::insert_request(DB_Request& request)
{
	request_db_queue.emplace(request); // DB 요청을 여기에 추가합니다.
}

void DataBase::DataBaseThread()
{
	while (true)
	{
		if (!request_db_queue.empty())
		{
			DB_Request request = request_db_queue.front();
			switch (request.type)
			{
			case REQUEST_LOGIN:
			{
				cGameServer& server = cGameServer::GetInstance();
				int reason = 0;
				reason = check_login(request.request_name, request.request_pw);

				if (reason == 1) // reason 0 : id가 존재하지 않음 / reason 1 : 성공 / reason 2 : pw가 틀림
				{		
					server.m_clients[request.request_id].set_login_state(Y_LOGIN);
					server.m_clients[request.request_id].set_state(CLIENT_STATE::ST_LOBBY);
					server.m_clients[request.request_id].set_name(request.request_char_name);
					server.send_login_ok_packet(request.request_id);


					DB_Request req = request;
					req.type = REQUEST_LOAD_CUSTOMIZING;
					request_db_queue.emplace(req);
				}
				else
				{
					if (reason == 0)
						server.send_login_fail_packet(request.request_id, LOGIN_FAIL_REASON::INVALID_ID);
					else
						server.send_login_fail_packet(request.request_id, LOGIN_FAIL_REASON::WRONG_PW);
				}
				break;
			}

			case REQUEST_CREATE_ID:
			{
				cGameServer& server = cGameServer::GetInstance();
				int reason = 0;
				reason = create_id(request.request_name, request.request_pw);

				if (reason == 1) // id 생성 성공
				{
					Custom init_data;
					init_data.body = MAINBODY01;
					init_data.body_parts = BODYPART01;
					init_data.eyes = EYE01;
					init_data.gloves = GLOVE01;
					init_data.head = HEAD01;
					init_data.mouthandnoses = MOUTH01;

					//m_database->Save_Customizing(stringToWstring(stringID), init_data);
					server.send_create_id_ok_packet(request.request_id);
					Save_Customizing(request.request_name, init_data);
				}
				else // id 생성 실패
					server.send_create_id_fail_packet(request.request_id, reason);
				break;
			}

			case REQUEST_LOAD_CUSTOMIZING:
			{
				cGameServer& server = cGameServer::GetInstance();
				Custom data = Load_Customizing(request.request_name);
				server.m_clients[request.request_id].m_customizing->Load_Customizing_Data_To_DB(data);
				
				//sc_packet_customizing_update packet;
				//packet.size = sizeof(packet);
				//packet.type = SC_PACKET::SC_PACKET_CUSTOMIZING;
				//packet.id = request.request_id;
				//packet.body = data.body;
				//packet.body_parts = data.body_parts;
				//packet.eyes = data.eyes;
				//packet.gloves = data.gloves;
				//packet.head = data.head;
				//packet.mouthandnoses = data.mouthandnoses;
				//server.m_clients[request.request_id].do_send(sizeof(packet), &packet);
				break;
			}

			case REQUEST_SAVE_CUSTOMIZING:
			{
				Save_Customizing(request.request_name, request.request_custom_data);
				cout << "커스터마이징 정보 저장 완료" << endl;
				break;
			}

			}
			request_db_queue.pop();
		}
	}
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