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

// Set to 1 to enable use of boost lockfree queue.
#define USE_BOOST_LOCKFREE_QUEUE 0 

#if USE_BOOST_LOCKFREE_QUEUE
#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/queue.hpp>
#else
#include <deque>
#endif

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
#if USE_BOOST_LOCKFREE_QUEUE
	typedef boost::lockfree::queue< class SysJob* > TJobQueue;
#else
	typedef std::deque< class SysJob* > TJobQueue;
#endif

	class SysKernel*		Parent_;
#if USE_BOOST_LOCKFREE_QUEUE
	TJobQueue				JobQueue_;
#else
	TJobQueue				JobQueue_;
	std::mutex				JobQueueMutex_;
#endif
	std::atomic< BcU32 >	NoofJobs_;
};

#endif
