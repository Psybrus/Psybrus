#include "MainShared.h"
#include <nl_types.h>
void engine_dummy()
{
}


// Unimplemented in Android, TODO own impls.
extern "C"
{
	nl_catd catopen(const char *name, int flag)
	{
		return nl_catd();
	}

	int catclose(nl_catd catalog)
	{
	}

	char *catgets(nl_catd catalog, int set_number, int message_number, const char *message)
	{
		return nullptr;
	}
}