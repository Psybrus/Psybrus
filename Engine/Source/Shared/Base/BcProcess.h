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
 * @param Path Path to executable.
 * @param Arguments Arguments for executable.
 * @return Future with return code.
 */
std::future< int > BcProcessLaunch(
	const char* Path,
	const char* Arguments,
	BcProcessLogFunc LogFunc );
