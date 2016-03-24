#pragma once

#include "Base/BcTypes.h"

#include <string>
#include <vector>

class BcCommandLine
{
public:
	BcCommandLine();

	/**
	 * Construct with expanded command line.
	 * i.e.
	 * Game.exe -p parameter1 --long-parameter parameter2 "parameter 3"
	 */
	BcCommandLine( const char* CmdLine );	

	/**
	 * Construct with standard command line.
	 */
	BcCommandLine( int Argc, const char* Argv[] );	

	/**
	 * @param Short Short form of parameter, i.e. 'p'.
	 * @param Long Long form of parameter, i.e. 'param'
	 * @return Do we have an argument?
	 */
	bool hasArg( const char Short, const char* Long ) const;

	/**
	 * @param Short Short form of parameter, i.e. 'p'.
	 * @param Long Long form of parameter, i.e. 'param'
	 * @param Out Output argument.
	 * @return Do we have argument? Will return false if there is no argument after short/long form.
	 */
	bool getArg( const char Short, const char* Long, std::string& Out ) const;

	/**
	 * @return Command line as a c string.
	 */
	const char* c_str() const { return FullCommandline_.c_str(); }

private:
	std::string FullCommandline_;
	std::vector< std::string > Args_;
};
