#ifndef __DSCORELOGGING_H__
#define __DSCORELOGGING_H__

#include "Base/BcGlobal.h"
#include "Base/BcHtml.h"
#include "System/SysSystem.h"


#include <functional>
#include <map>


//////////////////////////////////////////////////////////////////////////
/**	\class DsCoreLogging
*	\brief Debug System logging code
*
*	Debugging system core.
*/

struct DsCoreLogEntry
{
public:
	DsCoreLogEntry(BcU32 TimeStamp, const std::string& Category, std::string Entry);
	BcU32 Timestamp_;
	std::string Entry_;
	std::string Category_;
};


class DsCoreLogging :
	public BcGlobal< DsCoreLogging >,
	public SysSystem
	{
public:

public:
	DsCoreLogging();
	virtual ~DsCoreLogging();
	virtual void addLog( const std::string& Category, BcU32 Timestamp, std::string Message ) = 0;
	virtual std::vector<DsCoreLogEntry> getEntries( std::string* Categories, BcU32 CategoryCount ) = 0;
private:

};



#endif
