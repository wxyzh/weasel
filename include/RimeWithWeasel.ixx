module;
#include <Windows.h>
#include <WeaselUI.h>
#include <rime_api.h>
export module RimeWithWeasel;
import <map>;
import <string>;
import <utility>;
import <cmath>;
import <regex>;
import <thread>;
import WeaselIPC;

export
{
	struct CaseInsensitiveCompare
	{
		bool operator()(std::string_view lhs, std::string_view rhs) const
		{
			std::string str1, str2;
			std::transform(lhs.begin(), lhs.end(), std::back_inserter(str1),
				[](char c) { return std::tolower(c); });
			std::transform(rhs.begin(), rhs.end(), std::back_inserter(str2),
				[](char c) { return std::tolower(c); });
			return str1 < str2;
		}
	};

	using AppOptions = std::map<std::string, bool>;
	using AppOptionsByAppName = std::map<std::string, AppOptions, CaseInsensitiveCompare>;
	struct SessionStatus
	{
		SessionStatus() { RIME_STRUCT(RimeStatus, status); }
		weasel::UIStyle style{};
		bool __synced{};
		RimeStatus status;		
	};
	using SessionStatusMap = std::map<RimeSessionId, SessionStatus>;

	class RimeWithWeaselHandler :
		public weasel::RequestHandler
	{
	public:
		RimeWithWeaselHandler(weasel::UI* ui);
		virtual ~RimeWithWeaselHandler();
		virtual void Initialize() override;
		virtual void Finalize() override;
		virtual RimeSessionId FindSession(RimeSessionId session_id) override;
		virtual RimeSessionId AddSession(LPWSTR buffer, EatLine eat = 0) override;
		virtual RimeSessionId RemoveSession(RimeSessionId session_id) override;
		virtual BOOL ProcessKeyEvent(weasel::KeyEvent keyEvent, RimeSessionId session_id, EatLine eat) override;
		virtual void CommitComposition(RimeSessionId session_id) override;
		virtual void ClearComposition(RimeSessionId session_id) override;
		virtual void SelectCandidateOnCurrentPage(const size_t index, RimeSessionId session_id) override;
		virtual void FocusIn(PARAM param, RimeSessionId session_id) override;
		virtual void FocusOut(PARAM param, RimeSessionId session_id) override;
		virtual void UpdateInputPosition(RECT const& rc, RimeSessionId session_id) override;
		virtual void StartMaintenance() override;
		virtual void EndMaintenance() override;
		virtual void SetOption(RimeSessionId session_id, const std::string& opt, bool val) override;
		virtual void UpdateColorTheme(bool darkMode) override;

		void OnUpdateUI(std::function<void()> const& cb);

	private:
		void _Setup();
		bool _IsDeployerRunning();
		void _UpdateUI(RimeSessionId session_id);
		void _LoadSchemaSpecificSettings(RimeSessionId session_id, const std::string& schema_id);
		void _LoadIconSettingFromSchema(RimeConfig& config, char* buffer, const int BUF_SIZE,
			const char* key1, const char* key2, std::wstring_view user_dir, std::wstring_view shared_dir, std::wstring& value);
		void _LoadAppInlinePreeditSet(RimeSessionId session_id, bool ignore_app_name = false);
		bool _ShowMessage(weasel::Context& ctx, weasel::Status& status);
		bool _Respond(RimeSessionId session_id, EatLine eat);
		void _ReadClientInfo(RimeSessionId session_id, LPWSTR buffer);
		void _GetCandidateInfo(weasel::CandidateInfo& cinfo, RimeContext& ctx);
		void _GetStatus(weasel::Status& stat, RimeSessionId session_id, weasel::Context& ctx);
		void _GetContext(weasel::Context& ctx, RimeSessionId session_id);
		void _UpdateShowNotifications(RimeConfig* config, bool initialize = false);

		bool _IsSessionTSF(RimeSessionId session_id);
		void _UpdateInlinePreeditStatus(RimeSessionId session_id);

		AppOptionsByAppName m_app_options;
		weasel::UI* m_ui;  // reference
		RimeSessionId m_active_session;
		bool m_disabled;
		std::string m_last_schema_id;
		std::string m_last_app_name;
		weasel::UIStyle m_base_style;
		UINT m_show_notifications_when;
		std::map<std::string, bool> m_show_notifications;
		std::map<std::string, bool> m_show_notifications_base;
		std::function<void()> _UpdateUICallback;

		static void OnNotify(void* context_object,
			RimeSessionId session_id,
			const char* message_type,
			const char* message_value);
		static std::string m_message_type;
		static std::string m_message_value;
		static std::string m_message_label;
		static std::string m_option_name;
		SessionStatusMap m_session_status_map;		
		// std::map<RimeSessionId, bool> m_color_sync;
		bool m_current_dark_mode;
		bool m_global_ascii_mode;
		int m_show_notifications_time;
		bool m_add_session{};
	};
}