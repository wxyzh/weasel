module;
#include <rime_levers_api.h>
#pragma warning(disable:4005)
export module UIStyleSettings;
import <string>;
import <vector>;

export
{
	struct ColorSchemeInfo {
		std::string color_scheme_id;
		std::string name;
		std::string author;
	};

	class UIStyleSettings {
	public:
		UIStyleSettings();

		bool GetPresetColorSchemes(std::vector<ColorSchemeInfo>* result);
		std::string GetColorSchemePreview(const std::string& color_scheme_id);
		std::string GetActiveColorScheme();
		bool SelectColorScheme(const std::string& color_scheme_id);

		RimeCustomSettings* settings() { return settings_; }

	private:
		RimeLeversApi* api_;
		RimeCustomSettings* settings_;
	};
}
