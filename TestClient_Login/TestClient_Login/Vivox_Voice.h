#pragma once
#include "SDK/include/vivox-config.h"
#include "SDK/include/Vxc.h"
#include "SDK/include/VxcErrors.h"
#include "SDK/include/VxcEvents.h"
#include "SDK/include/VxcExports.h"
#include "SDK/include/VxcRequests.h"
#include "SDK/include/VxcResponses.h"
#include "SDK/include/VxcTypes.h"

class Vivox_Voice {
private:
	int									status;
	vx_sdk_config_t						defaultConfig;
	vx_req_account_anonymous_login_t*	req;

public:
	Vivox_Voice();
	~Vivox_Voice();

	bool init_Vivox();
	void create_connect_object();
	bool Start_Login_Process();

	void HandleLoginResponse(vx_resp_account_anonymous_login* resp);
	void HandleLoginStateChange(vx_evt_account_login_state_change* evt);

	void MyGamesResponseHandler(vx_resp_base_t* resp);

	static vx_sdk_config_t MakeConfig();
};