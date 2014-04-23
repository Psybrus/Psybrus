/**************************************************************************
*
* File:		SysJobQueue.h
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SysJobQueue_H__
#define __SysJobQueue_H__

#include "Base/BcTypes.h"
#include "Base/BcMisc.h"

#include "System/SysJob.h"
#include "System/SysFence.h"

#include <thread>
#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/queue.hpp>

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class SysJobQueue;

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef std::vector< SysJobQueue* > SysJobQueueList;

//////////////////////////////////////////////////////////////////////////
// SysJobQueue
class SysJobQueue
{
public:
	SysJobQueue( class SysKernel* Parent );
	virtual ~SysJobQueue();
	
	/**
	 * Push a job.
	 * Thread safe.
	 * @param Job Job to push.
	 * @return true if successfully queued.
	 */
	BcBool				pushJob( SysJob* Job );
	
	/**
	 * Pop a job.
	 * Thread safe.
	 * @param Job Job we have popped.
	 * @return true if successfully popped.
	 */
	BcBool				popJob( SysJob*& Job );

	/**
	 * Flush jobs.
	 * @param ForceExecute Force execute on this call.
	 */
	void				flushJobs( BcBool ForceExecute );

	/**
	 * Do we have jobs?
	 */
	BcBool				anyJobsPending();
	
private:
	typedef boost::lockfree::queue< class SysJob* > TJobQueue;
	
	class SysKernel*		Parent_;
	TJobQueue				JobQueue_;
	std::atomic< BcU32 >	NoofJobs_;
};

#endif
