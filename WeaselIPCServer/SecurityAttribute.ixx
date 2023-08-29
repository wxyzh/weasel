module;
#include "stdafx.h"
#include <winnt.h> // for security attributes constants
#include <aclapi.h> // for ACL
export module SecurityAttribute;

export namespace weasel
{
	class SecurityAttribute 
	{
	private:
		PSECURITY_DESCRIPTOR pd;
		SECURITY_ATTRIBUTES sa;
		void _Init();
	public:
		SecurityAttribute() : pd(NULL) { _Init(); }
		SECURITY_ATTRIBUTES *get_attr();
	};
};
