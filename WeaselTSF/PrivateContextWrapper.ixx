module;
#include "stdafx.h"
export module PrivateContextWrapper;

export
{
	class PrivateContextWrapper final
	{
	public:
		PrivateContextWrapper() = delete;
		PrivateContextWrapper(const PrivateContextWrapper&) = delete;
		PrivateContextWrapper& operator=(const PrivateContextWrapper&) = delete;

		explicit PrivateContextWrapper(std::function<void()> sink_cleaner)
			: sink_cleaner_{ sink_cleaner }{}

		~PrivateContextWrapper() { sink_cleaner_(); }

	private:
		std::function<void()> sink_cleaner_;
	};
}