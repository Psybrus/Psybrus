#ifndef __DSCORELOGGINGIMPL_H__
#define __DSCORELOGGINGIMPL_H__

#include "Base/BcGlobal.h"
#include "Base/BcHtml.h"
#include "System/SysSystem.h"
#include "System/Debug/DsCoreLogging.h"

#include <functional>
#include <map>

//////////////////////////////////////////////////////////////////////////
/**	\class DsCoreLoggingImpl
*	\brief Debug System logging implementation
*
*	Debugging system core.
*/
class DsCoreLoggingImpl :
	public DsCoreLogging
{
public:
	virtual void open();
	virtual void update();
	virtual void close();

public:
	DsCoreLoggingImpl();
	virtual ~DsCoreLoggingImpl();
	virtual void addLog( const std::string& Category, BcU32 Timestamp, std::string Message );
	virtual std::vector< DsCoreLogEntry > getEntries( std::string* Categories, BcU32 CategoryCount );
	
private:
	std::map< std::string, std::deque< DsCoreLogEntry > > Logs_;
	std::mutex Lock_;
};



#endif
