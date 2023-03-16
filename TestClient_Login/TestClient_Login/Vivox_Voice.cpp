#include <iostream>
#include "Vivox_Voice.h"

Vivox_Voice::Vivox_Voice()
{

}

Vivox_Voice::~Vivox_Voice()
{

}

bool Vivox_Voice::init_Vivox()
{
	status = vx_get_default_config3(&defaultConfig, sizeof(defaultConfig));

	if (status != VX_E_SUCCESS)
	{
		std::cout << "vx_sdk_get_default_config3() returned " << status << ": " << vx_get_error_string(status) << std::endl;
		return false;
	}

	status = vx_initialize3(&defaultConfig, sizeof(defaultConfig));
	if (status != VX_E_SUCCESS)
	{
		std::cout << "vx_initialize3() returned " << status << ": " << vx_get_error_string(status) << std::endl;
		return false;
	}
	return true;  // 초기화 성공을 알림
}

void Vivox_Voice::create_connect_object()
{
	vx_req_connector_create* request;
	vx_req_connector_create_create(&request);

	request->connector_handle = vx_strdup("c1");
	req->acct_mgmt_server = vx_strdup("https://mt1s.www.vivox.com/api2");
	
	int request_count;
	int vx_issue_request3_response = vx_issue_request3(&request->base, &request_count);
}

bool Vivox_Voice::Start_Login_Process()
{
	vx_req_account_anonymous_login_t* req;
	vx_req_account_anonymous_login_create(&req);
	req->connector_handle = vx_strdup("c1");
	req->acct_name = vx_strdup("woobin2619-gr35-dev");
	req->displayname = vx_strdup("kim woo bin");
	req->account_handle = vx_strdup(req->acct_name);
	req->access_token = vx_strdup("zoom905");

	status = vx_issue_request(&req->base);

	if (status != VX_E_SUCCESS)
		return false;
	return true;
}

void Vivox_Voice::HandleLoginResponse(vx_resp_account_anonymous_login* resp)
{
	if (resp->base.return_code == 1)
	{
		std::cout << resp->base.status_code << " : " << vx_get_error_string(resp->base.status_code) << std::endl;
		return;
	}
	vx_req_account_anonymous_login* req = reinterpret_cast<vx_req_account_anonymous_login*>(req->base.type);
	printf("login succeeded for account %s\n", req->acct_name);
}

void Vivox_Voice::HandleLoginStateChange(vx_evt_account_login_state_change* evt)
{
	if (evt->state == login_state_logged_in)
	{
		printf("%s is logged in\n", evt->account_handle);
	}
	else if (evt->state == login_state_logged_out)
	{
		if (evt->status_code != 0)
		{
			printf("%s logged out with status %d:%s\n", evt->status_code, vx_get_error_string(evt->status_code));
		}
		else
		{
			;
		}
	}
}


void Vivox_Voice::MyGamesResponseHandler(vx_resp_base_t* resp)
{
	switch (resp->type)
	{
	case resp_connector_create:

		break;

	case resp_account_anonymous_login:

		break;
	}
}