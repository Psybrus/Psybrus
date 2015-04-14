/**************************************************************************
*
* File:		SysJob.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJob.h"

#define DEBUG_JOB_IDS ( 0 )

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJob::SysJob():
	WorkerMask_( 0 )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
// virtual
SysJob::~SysJob()
{
	
}

//////////////////////////////////////////////////////////////////////////
// internalExecute
// virtual
void SysJob::internalExecute()
{
	execute();
}

//////////////////////////////////////////////////////////////////////////
// SysFunctionJob Statics
std::atomic< size_t > SysFunctionJob::FunctionJobID_( 0 );

//////////////////////////////////////////////////////////////////////////
// Ctor
SysFunctionJob::SysFunctionJob( std::function< void() > Function ) :
	Function_( Function ),
	JobID_( FunctionJobID_++ )
{
#if DEBUG_JOB_IDS
	PSY_LOG( "Created job %u", JobID_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysFunctionJob::~SysFunctionJob()
{
	
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysFunctionJob::execute()
{
#if DEBUG_JOB_IDS
	PSY_LOG( "Running job %u", JobID_ );
#endif
	Function_( );
#if DEBUG_JOB_IDS
	PSY_LOG( "Ending job %u", JobID_ );
#endif
}
