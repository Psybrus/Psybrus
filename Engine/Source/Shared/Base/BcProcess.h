#pragma once

#include "Base/BcTypes.h"

#include <functional>
#include <future>

/**
 * Typedef for logging function.
 */
typedef std::function< void( const char* ) > BcProcessLogFunc;

/**
 * Launch process.
 * Calls to @a Logfunc come from a background thread.
 * @param Commandline Command line to launch.
 * @return Future with return code.
 */
std::future< int > BcProcessLaunch(
	const char* Commandline,
	BcProcessLogFunc LogFunc );
