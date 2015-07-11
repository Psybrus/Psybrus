#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// Dummy entrypoint.
void dummyEntry()
{
};

//////////////////////////////////////////////////////////////////////////
// Android entry point
#if PLATFORM_ANDROID
#include <android_native_app_glue.h>

extern void PsyAndroidMain( struct android_app* State );

extern "C"
{
	void android_main( struct android_app* State )
	{
		// Make sure glue isn't stripped.
		app_dummy();

		// Make sure game isn't stripped.
		extern void game_dummy();
		game_dummy();

		//PsyAndroidMain( State );
	}
}
#endif

