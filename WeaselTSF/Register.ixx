module;
#include "Globals.h"
export module Register;

export
{
	BOOL RegisterProfiles();
	void UnregisterProfiles();
	BOOL RegisterCategories();
	void UnregisterCategories();
	BOOL RegisterServer();
	void UnregisterServer();
}