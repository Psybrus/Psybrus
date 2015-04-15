#include "System/Debug/DsCoreLogging.h"
#include "System/SysKernel.h"

#include "Psybrus.h"

DsCoreLogEntry::DsCoreLogEntry(BcU32 TimeStamp, const std::string& Category, std::string Entry)
	: Timestamp_(TimeStamp), Category_(Category), Entry_(Entry)
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCoreLogging::DsCoreLogging()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCoreLogging::~DsCoreLogging()
{

}

