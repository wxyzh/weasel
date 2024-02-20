module;
#include "stdafx.h"
#include <WeaselCommon.h>
#include <WeaselVersion.h>
#include "WeaselDeployer.h"
#include "UIStyleSettings.h"
#include "UIStyleSettingsDialog.h"
#include "ConfiguratorDialog.h"
#include "SwitcherSettingsDialog.h"
#include "DictManagementDialog.h"
#include "resource.h"
#include <rime_api.h>
#include <rime_levers_api.h>
#pragma warning(default: 4005)
#pragma comment(lib, "rime.lib")
module Config;
import WeaselIPC;
import WeaselUtility;

namespace fs = std::filesystem;

Configurator::Configurator()
{
	create_miss_file();
}

void Configurator::Initialize()
{
	RIME_STRUCT(RimeTraits, weasel_traits);
	auto shared_dir{ weasel_shared_data_dir() };
	auto user_dir{ weasel_user_data_dir() };
	auto name{ to_string(WEASEL_IME_NAME, CP_UTF8) };
	auto log_dir{ weasel_log_dir() };
	weasel_traits.shared_data_dir = shared_dir.data();
	weasel_traits.user_data_dir = user_dir.data();
	weasel_traits.prebuilt_data_dir = shared_dir.data();
	weasel_traits.distribution_name = name.data();
	weasel_traits.distribution_code_name = WEASEL_CODE_NAME;
	weasel_traits.distribution_version = WEASEL_VERSION;
	weasel_traits.app_name = "rime.weasel";
	weasel_traits.log_dir = log_dir.data();
	RimeSetup(&weasel_traits);
	
	LOG(INFO) << "WeaselDeployer reporting.";
	RimeDeployerInitialize(NULL);
}

static bool configure_switcher(RimeLeversApi* api, RimeSwitcherSettings* switcher_settings, bool* reconfigured)
{
	RimeCustomSettings* settings = (RimeCustomSettings*)switcher_settings;
    if (!api->load_settings(settings))
        return false;
	bool selected{ true };
	SwitcherSettingsDialog dialog(switcher_settings, selected);
	if (dialog.DoModal() == IDOK) {
		if (api->save_settings(settings))
			*reconfigured = true;
		return true;
	}
	return false;
}

static bool configure_ui(RimeLeversApi* api, UIStyleSettings* ui_style_settings, bool* reconfigured) {
	RimeCustomSettings* settings = ui_style_settings->settings();
    if (!api->load_settings(settings))
        return false;
	UIStyleSettingsDialog dialog(ui_style_settings);
	if (dialog.DoModal() == IDOK) {
		if (api->save_settings(settings))
			*reconfigured = true;
		return true;
	}
	return false;
}

static bool configure(RimeLeversApi* api, RimeSwitcherSettings* switcher_settings, UIStyleSettings* ui_style_settings, bool& reconfigured)
{
	RimeCustomSettings* settings = (RimeCustomSettings*)switcher_settings;
	if (!api->load_settings(settings))
		return false;
	RimeCustomSettings* style_settings = ui_style_settings->settings();
	if (!api->load_settings(style_settings))
		return false;

	ConfiguratorDialog dialog(switcher_settings, ui_style_settings);
	if (dialog.DoModal() == IDOK)
	{
		if (api->save_settings(settings) || api->save_settings(style_settings))
		{
			reconfigured = true;
			return true;
		}
		reconfigured = false;
	}
	return false;
}

int Configurator::Run(bool installing)
{
	RimeModule* levers = rime_get_api()->find_module("levers");
	if (!levers) return 1;
	RimeLeversApi* api = (RimeLeversApi*)levers->get_api();
	if (!api) return 1;

	bool reconfigured = false;

	RimeSwitcherSettings* switcher_settings = api->switcher_settings_init();
	UIStyleSettings ui_style_settings;

	bool skip_switcher_settings = installing && !api->is_first_run((RimeCustomSettings*)switcher_settings);
	bool skip_ui_style_settings = installing && !api->is_first_run(ui_style_settings.settings());

	/*(skip_switcher_settings || configure_switcher(api, switcher_settings, &reconfigured)) &&
		(skip_ui_style_settings || configure_ui(api, &ui_style_settings, &reconfigured));*/

	skip_switcher_settings && skip_ui_style_settings || configure(api, switcher_settings, &ui_style_settings, reconfigured);

	api->custom_settings_destroy((RimeCustomSettings*)switcher_settings);

	if (installing || reconfigured) {
		return UpdateWorkspace(reconfigured);
	}
	return 0;
}

int Configurator::UpdateWorkspace(bool report_errors) {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"WeaselDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating WeaselDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		if (report_errors)
		{
			MessageBox(NULL, L"正在执行另一项部署任务，方才所做的修改将在输入法再次启动后生效。", L"【小狼毫】", MB_OK | MB_ICONINFORMATION);
		}
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		// initialize default config, preset schemas
		rime->deploy();
		// initialize weasel config
		rime->deploy_config_file("weasel.yaml", "config_version");
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}

int Configurator::DictManagement() {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"WeaselDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating WeaselDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		MessageBox(NULL, L"正在执行另一项部署任务，请稍候再试。", L"【小狼毫】", MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		if (RIME_API_AVAILABLE(rime, run_task))
		{
			rime->run_task("installation_update");  // setup user data sync dir
		}
		DictManagementDialog dlg;
		dlg.DoModal();
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}

int Configurator::SyncUserData() {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"WeaselDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating WeaselDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		MessageBox(NULL, L"正在执行另一项部署任务，请稍候再试。", L"【小狼毫】", MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		if (!rime->sync_user_data())
		{
			LOG(ERROR) << "Error synching user data.";
			CloseHandle(hMutex);
			return 1;
		}
		rime->join_maintenance_thread();
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}

void Configurator::create_miss_file()
{
	auto user_data_dir{ WeaselUserDataPath() };
	
	fs::path default_custom{ std::format(LR"({}\default.custom.yaml)", user_data_dir) };
	if (!fs::exists(default_custom))
	{
		std::wofstream out{ default_custom, std::ios::app };
	}

	fs::path weasel_custom{ std::format(LR"({}\weasel.custom.yaml)", user_data_dir) };
	if (!fs::exists(weasel_custom))
	{
		std::wofstream out{ weasel_custom, std::ios::app };
	}
}