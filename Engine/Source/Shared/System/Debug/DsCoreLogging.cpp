#include "System/Debug/DsCoreLogging.h"
#include "System/SysKernel.h"

#include "Psybrus.h"

DsCoreLogEntry::DsCoreLogEntry(BcU32 TimeStamp, BcName Category, std::string Entry)
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

