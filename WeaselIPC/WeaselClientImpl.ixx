module;
#include "stdafx.h"
#include <Windows.h>
export module WeaselClientImpl;
import <string>;
import WeaselIPC;
import PipeChannel;


export namespace weasel
{
	class ClientImpl
	{
	public:
		ClientImpl();
		~ClientImpl();

		bool Connect(ServerLauncher const& launcher);
		void Disconnect();
		void ShutdownServer();
		void StartSession();
		void EndSession();
		void StartMaintenance();
		void EndMaintenance();
		bool Echo();
		bool ProcessKeyEvent(KeyEvent const& keyEvent);
		bool CommitComposition();
		bool ClearComposition();
		bool SelectCandidateOnCurrentPage(const size_t index);
		void UpdateInputPosition(RECT const& rc);
		void FocusIn();
		void FocusOut();
		void TrayCommand(PARAM menuId);
		bool GetResponseData(ResponseHandler const& handler);

	protected:
		void _InitializeClientInfo();
		bool _WriteClientInfo();

		PARAM _SendMessage(WEASEL_IPC_COMMAND Msg, PARAM wParam, RimeSessionId lParam);

		bool _Connected() const { return channel.Connected(); }
		bool _Active() const { return channel.Connected() && session_id != 0; }

	private:
		RimeSessionId session_id;
		std::wstring app_name;
		bool is_ime;

		PipeChannel<PipeMessage> channel;
	};

}