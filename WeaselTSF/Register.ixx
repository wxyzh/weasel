module;
#include "Globals.h"
export module Register;

export
{
	HKL FindIME();
	BOOL RegisterProfiles();
	void UnregisterProfiles();
	BOOL RegisterCategories();
	void UnregisterCategories();
	BOOL RegisterServer();
	void UnregisterServer();
}