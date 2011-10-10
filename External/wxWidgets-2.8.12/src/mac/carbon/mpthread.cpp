/////////////////////////////////////////////////////////////////////////////
// Name:      src/mac/carbon/mpthread.cpp
// Purpose:   wxThread Implementation
// Author:    Original from Wolfram Gloger/Guilhem Lavaux/Vadim Zeitlin
// Modified   by: Aj Lavin, Stefan Csomor
// Created:   04/22/98
// RCS-ID:    $Id: mpthread.cpp 40943 2006-08-31 19:31:43Z ABX $
// Copyright: (c) Wolfram Gloger (1996, 1997); Guilhem Lavaux (1998),
//                Vadim Zeitlin (1999) , Stefan Csomor (2000)
// Licence:   wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#if wxUSE_THREADS

#ifndef WX_PRECOMP
    #include "wx/wx.h"
    #include "wx/module.h"
#endif

#include "wx/thread.h"

#ifdef __WXMAC__
#if TARGET_API_MAC_OSX
#include <CoreServices/CoreServices.h>
#else
#include <DriverServices.h>
#include <Multiprocessing.h>
#endif
#include "wx/mac/uma.h"
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the possible states of the thread ("=>" shows all possible transitions from
// this state)
enum wxThreadState
{
    STATE_NEW,      // didn't start execution yet (=> RUNNING)
    STATE_RUNNING,  // thread is running (=> PAUSED, CANCELED)
    STATE_PAUSED,   // thread is temporarily suspended (=> RUNNING)
    STATE_CANCELED, // thread should terminate a.s.a.p. (=> EXITED)
    STATE_EXITED    // thread is terminating
};

// ----------------------------------------------------------------------------
// this module globals
// ----------------------------------------------------------------------------


// the task ID of the main thread
static wxThreadIdType gs_idMainThread = kInvalidID;

// this is the Per-Task Storage for the pointer to the appropriate wxThread
TaskStorageIndex gs_tlsForWXThread = 0 ;

// if it's false, some secondary thread is holding the GUI lock
static bool gs_bGuiOwnedByMainThread = true;

// critical section which controls access to all GUI functions: any secondary
// thread (i.e. except the main one) must enter this crit section before doing
// any GUI calls
static wxCriticalSection *gs_critsectGui = NULL;

// critical section which protects gs_nWaitingForGui variable
static wxCriticalSection *gs_critsectWaitingForGui = NULL;

// number of threads waiting for GUI in wxMutexGuiEnter()
static size_t gs_nWaitingForGui = 0;

// overall number of threads, needed for determining the sleep value of the main
// event loop
size_t g_numberOfThreads = 0;



#if wxUSE_GUI

MPCriticalRegionID gs_guiCritical = kInvalidID;

#endif

// ============================================================================
// MacOS implementation of thread classes
// ============================================================================

/*
    Notes :

    The implementation is very close to the phtreads implementation, the reason for
    using MPServices is the fact that these are also available under OS 9. Thus allowing
    for one common API for all current builds.

    As soon as wxThreads are on a 64 bit address space, the TLS must be extended
    to use two indices one for each 32 bit part as the MP implementation is limited
    to longs.

    I have two implementations for mutexes :
    version A based on a binary semaphore, problem - not reentrant, version B based
    on a critical region, allows for reentrancy, performance implications not
    yet tested

    The same for condition internal, one implementation by Aj Lavin and the other one
    copied from the thrimpl.cpp which I assume has been more broadly tested, I've just
    replaced the interlock increment with the appropriate PPC calls
*/

// ----------------------------------------------------------------------------
// wxMutex implementation
// ----------------------------------------------------------------------------

static bool wxMacMPThreadsInitVerify()
{
    static bool hasThreadManager = false ;
    if ( !hasThreadManager )
        hasThreadManager = MPLibraryIsLoaded();

    if ( !hasThreadManager )
    {
        wxMessageBox( wxT("Error") , wxT("MP Thread Support is not available on this System" ), wxOK ) ;
        return false ;
    }
    return true ;
}

#if 0

class wxMutexInternal
{
public:
    wxMutexInternal(wxMutexType mutexType) ;
    ~wxMutexInternal() ;
    bool IsOk() const { return m_isOk; }

    wxMutexError Lock() ;
    wxMutexError TryLock() ;
    wxMutexError Unlock();
private:
    MPSemaphoreID m_semaphore;
    bool m_isOk ;
};

wxMutexInternal::wxMutexInternal(wxMutexType mutexType )
{
    wxMacMPThreadsInitVerify() ;

    m_isOk = false ;
    m_semaphore = kInvalidID ;

    OSStatus err = noErr ;
    switch( mutexType )
    {
        case wxMUTEX_DEFAULT :
            {
                verify_noerr( MPCreateBinarySemaphore( & m_semaphore) );
                m_isOk = ( m_semaphore != kInvalidID ) ;
            }
            break ;
        case wxMUTEX_RECURSIVE :
            wxFAIL_MSG(wxT("Recursive Mutex not supported yet") ) ;
            break ;
        default :
            wxFAIL_MSG(wxT("Unknown mutex type") ) ;
            break ;
    }
}

wxMutexInternal::~wxMutexInternal()
{
    if ( m_semaphore != kInvalidID )
        MPDeleteSemaphore( m_semaphore);
}

wxMutexError wxMutexInternal::Lock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPWaitOnSemaphore( m_semaphore, kDurationForever);
    if ( err)
    {
        wxLogSysError(wxT("Could not lock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

wxMutexError wxMutexInternal::TryLock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPWaitOnSemaphore( m_semaphore, kDurationImmediate);
    if ( err)
    {
        if ( err == kMPTimeoutErr)
        {
            return wxMUTEX_BUSY;
        }
        wxLogSysError(wxT("Could not try lock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

wxMutexError wxMutexInternal::Unlock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPSignalSemaphore( m_semaphore);
    if ( err)
    {
        wxLogSysError(_("Could not unlock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

#else

class wxMutexInternal
{
public:
    wxMutexInternal(wxMutexType mutexType) ;
    ~wxMutexInternal() ;
    bool IsOk() const { return m_isOk; }

    wxMutexError Lock() ;
    wxMutexError TryLock() ;
    wxMutexError Unlock();
private:
    MPCriticalRegionID m_critRegion ;
    bool m_isOk ;
};

wxMutexInternal::wxMutexInternal(wxMutexType mutexType )
{
    wxMacMPThreadsInitVerify() ;
    m_isOk = false ;
    m_critRegion = kInvalidID ;

    verify_noerr( MPCreateCriticalRegion( & m_critRegion) );
    m_isOk = ( m_critRegion != kInvalidID ) ;

    if ( !IsOk() )
        wxFAIL_MSG(wxT("Error when creating mutex") ) ;
}

wxMutexInternal::~wxMutexInternal()
{
    if ( m_critRegion != kInvalidID )
        MPDeleteCriticalRegion( m_critRegion);
}

wxMutexError wxMutexInternal::Lock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPEnterCriticalRegion( m_critRegion, kDurationForever);
    if ( err)
    {
        wxLogSysError(wxT("Could not lock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

wxMutexError wxMutexInternal::TryLock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPEnterCriticalRegion( m_critRegion, kDurationImmediate);
    if ( err)
    {
        if ( err == kMPTimeoutErr)
        {
            return wxMUTEX_BUSY;
        }
        wxLogSysError(wxT("Could not try lock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

wxMutexError wxMutexInternal::Unlock()
{
    wxCHECK_MSG( m_isOk , wxMUTEX_MISC_ERROR , wxT("Invalid Mutex") ) ;
    OSStatus err = MPExitCriticalRegion( m_critRegion);
    if ( err)
    {
        wxLogSysError(_("Could not unlock mutex"));
        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}

#endif

// --------------------------------------------------------------------------
// wxSemaphore
// --------------------------------------------------------------------------

class wxSemaphoreInternal
{
public:
    wxSemaphoreInternal(int initialcount, int maxcount);
    ~wxSemaphoreInternal();

    bool IsOk() const { return m_isOk; }

    wxSemaError WaitTimeout(unsigned long milliseconds);

    wxSemaError Wait() { return WaitTimeout( kDurationForever); }

    wxSemaError TryWait()
    {
        wxSemaError err = WaitTimeout(kDurationImmediate);
        if ( err == wxSEMA_TIMEOUT )
            err = wxSEMA_BUSY ;
        return err ;
    }
    wxSemaError Post();

private:
    MPSemaphoreID m_semaphore;
    bool m_isOk ;
};

wxSemaphoreInternal::wxSemaphoreInternal(int initialcount, int maxcount)
{
    wxMacMPThreadsInitVerify() ;
    m_isOk = false ;
    m_semaphore = kInvalidID ;
    if ( maxcount == 0 )
    {
        // make it practically infinite
        maxcount = INT_MAX;
    }
    verify_noerr( MPCreateSemaphore( maxcount, initialcount, & m_semaphore) );
    m_isOk = ( m_semaphore != kInvalidID ) ;

    if ( !IsOk() )
        wxFAIL_MSG(wxT("Error when creating semaphore") ) ;
}

wxSemaphoreInternal::~wxSemaphoreInternal()
{
    if( m_semaphore != kInvalidID )
        MPDeleteSemaphore( m_semaphore);
}

wxSemaError wxSemaphoreInternal::WaitTimeout(unsigned long milliseconds)
{
    OSStatus err = MPWaitOnSemaphore( m_semaphore, milliseconds);
    if ( err)
    {
        if ( err == kMPTimeoutErr)
        {
            return wxSEMA_TIMEOUT;
        }
        return wxSEMA_MISC_ERROR;
    }
    return wxSEMA_NO_ERROR;
}

wxSemaError wxSemaphoreInternal::Post()
{
    OSStatus err = MPSignalSemaphore( m_semaphore);
    if ( err)
    {
        return wxSEMA_MISC_ERROR;
    }
    return wxSEMA_NO_ERROR;
}

// ----------------------------------------------------------------------------
// wxCondition implementation
// ----------------------------------------------------------------------------

#if 0

class wxConditionInternal
{
public:

    wxConditionInternal(wxMutex& mutex)
    : m_mutex( mutex),
    m_semaphore( 0, 1),
    m_gate( 1, 1)
    {
        m_waiters = 0;
        m_signals = 0;
        m_canceled = 0;
    }

    ~wxConditionInternal()
    {
    }

    bool IsOk() const { return m_mutex.IsOk() ; }

    wxCondError Wait()
    {
        return WaitTimeout( kDurationForever);
    }

    wxCondError WaitTimeout(unsigned long msectimeout);

    wxCondError Signal()
    {
        return DoSignal( false);
    }

    wxCondError Broadcast()
    {
        return DoSignal( true);
    }

private:

    wxCondError DoSignal( bool signalAll);

    wxMutex&                m_mutex;
    wxSemaphoreInternal     m_semaphore;  // Signals the waiting threads.
    wxSemaphoreInternal     m_gate;
    wxCriticalSection       m_varSection;
    size_t                  m_waiters;    // Number of threads waiting for a signal.
    size_t                  m_signals;    // Number of signals to send.
    size_t                  m_canceled;   // Number of canceled waiters in m_waiters.
};


wxCondError wxConditionInternal::WaitTimeout(unsigned long msectimeout)
{
	m_gate.Wait();
	if ( ++ m_waiters == INT_MAX)
    {
		m_varSection.Enter();
		m_waiters -= m_canceled;
		m_signals -= m_canceled;
		m_canceled = 0;
		m_varSection.Leave();
    }
	m_gate.Post();

	m_mutex.Unlock();

	wxSemaError err = m_semaphore.WaitTimeout( msectimeout);
	wxASSERT( err == wxSEMA_NO_ERROR || err == wxSEMA_TIMEOUT);

	m_varSection.Enter();
	if ( err != wxSEMA_NO_ERROR)
    {
		if ( m_signals > m_canceled)
		{
			// A signal is being sent after we timed out.

			if ( m_waiters == m_signals)
			{
				// There are no excess waiters to catch the signal, so
				// we must throw it away.

				wxSemaError err2 = m_semaphore.Wait();
				if ( err2 != wxSEMA_NO_ERROR)
				{
					wxLogSysError(_("Error while waiting on semaphore"));
				}
				wxASSERT( err2 == wxSEMA_NO_ERROR);
				-- m_waiters;
				if ( -- m_signals == m_canceled)
				{
					// This was the last signal. open the gate.
					wxASSERT( m_waiters == m_canceled);
					m_gate.Post();
				}
			}
			else
			{
				// There are excess waiters to catch the signal, leave
				// it be.
				-- m_waiters;
			}
		}
		else
		{
			// No signals is being sent.
			// The gate may be open or closed, so we can't touch m_waiters.
			++ m_canceled;
			++ m_signals;
		}
    }
	else
    {
		// We caught a signal.
		wxASSERT( m_signals > m_canceled);
		-- m_waiters;
		if ( -- m_signals == m_canceled)
		{
			// This was the last signal. open the gate.
			wxASSERT( m_waiters == m_canceled);
			m_gate.Post();
		}
    }
	m_varSection.Leave();

	m_mutex.Lock();

    if ( err)
    {
		return err == wxSEMA_TIMEOUT ? wxCOND_TIMEOUT : wxCOND_MISC_ERROR;
    }

    return wxCOND_NO_ERROR;
}


wxCondError wxConditionInternal::DoSignal( bool signalAll)
{
	m_gate.Wait();
	m_varSection.Enter();

	wxASSERT( m_signals == m_canceled);

	if ( m_waiters == m_canceled)
    {
		m_varSection.Leave();
		m_gate.Post();
		return wxCOND_NO_ERROR;
    }

	if ( m_canceled > 0)
    {
		m_waiters -= m_canceled;
		m_signals = 0;
		m_canceled = 0;
    }

	m_signals = signalAll ? m_waiters : 1;
	size_t n = m_signals;

	m_varSection.Leave();

	// Let the waiters inherit the gate lock.

	do
    {
		wxSemaError err = m_semaphore.Post();
		wxASSERT( err == wxSEMA_NO_ERROR);
    } while ( -- n);

	return wxCOND_NO_ERROR;
}

#else
class wxConditionInternal
{
public:
    wxConditionInternal(wxMutex& mutex);

    bool IsOk() const { return m_mutex.IsOk() && m_semaphore.IsOk(); }

    wxCondError Wait();
    wxCondError WaitTimeout(unsigned long milliseconds);

    wxCondError Signal();
    wxCondError Broadcast();

private:
    // the number of threads currently waiting for this condition
    SInt32 m_numWaiters;

    // the critical section protecting m_numWaiters
    wxCriticalSection m_csWaiters;

    wxMutex& m_mutex;
    wxSemaphore m_semaphore;

    DECLARE_NO_COPY_CLASS(wxConditionInternal)
};

wxConditionInternal::wxConditionInternal(wxMutex& mutex)
                   : m_mutex(mutex)
{
    // another thread can't access it until we return from ctor, so no need to
    // protect access to m_numWaiters here
    m_numWaiters = 0;
}

wxCondError wxConditionInternal::Wait()
{
    // increment the number of waiters
    IncrementAtomic(&m_numWaiters);

    m_mutex.Unlock();

    // a potential race condition can occur here
    //
    // after a thread increments nwaiters, and unlocks the mutex and before the
    // semaphore.Wait() is called, if another thread can cause a signal to be
    // generated
    //
    // this race condition is handled by using a semaphore and incrementing the
    // semaphore only if 'nwaiters' is greater that zero since the semaphore,
    // can 'remember' signals the race condition will not occur

    // wait ( if necessary ) and decrement semaphore
    wxSemaError err = m_semaphore.Wait();
    m_mutex.Lock();

    return err == wxSEMA_NO_ERROR ? wxCOND_NO_ERROR : wxCOND_MISC_ERROR;
}

wxCondError wxConditionInternal::WaitTimeout(unsigned long milliseconds)
{
    IncrementAtomic(&m_numWaiters);

    m_mutex.Unlock();

    // a race condition can occur at this point in the code
    //
    // please see the comments in Wait(), for details

    wxSemaError err = m_semaphore.WaitTimeout(milliseconds);

    if ( err == wxSEMA_BUSY )
    {
        // another potential race condition exists here it is caused when a
        // 'waiting' thread timesout, and returns from WaitForSingleObject, but
        // has not yet decremented 'nwaiters'.
        //
        // at this point if another thread calls signal() then the semaphore
        // will be incremented, but the waiting thread will miss it.
        //
        // to handle this particular case, the waiting thread calls
        // WaitForSingleObject again with a timeout of 0, after locking
        // 'nwaiters_mutex'. this call does not block because of the zero
        // timeout, but will allow the waiting thread to catch the missed
        // signals.
        wxCriticalSectionLocker lock(m_csWaiters);

        err = m_semaphore.WaitTimeout(0);

        if ( err != wxSEMA_NO_ERROR )
        {
            m_numWaiters--;
        }
    }

    m_mutex.Lock();

    return err == wxSEMA_NO_ERROR ? wxCOND_NO_ERROR : wxCOND_MISC_ERROR;
}

wxCondError wxConditionInternal::Signal()
{
    wxCriticalSectionLocker lock(m_csWaiters);

    if ( m_numWaiters > 0 )
    {
        // increment the semaphore by 1
        if ( m_semaphore.Post() != wxSEMA_NO_ERROR )
            return wxCOND_MISC_ERROR;

        m_numWaiters--;
    }

    return wxCOND_NO_ERROR;
}

wxCondError wxConditionInternal::Broadcast()
{
    wxCriticalSectionLocker lock(m_csWaiters);

    while ( m_numWaiters > 0 )
    {
        if ( m_semaphore.Post() != wxSEMA_NO_ERROR )
            return wxCOND_MISC_ERROR;

        m_numWaiters--;
    }

    return wxCOND_NO_ERROR;
}
#endif

// ----------------------------------------------------------------------------
// wxCriticalSection implementation
// ----------------------------------------------------------------------------

// XXX currently implemented as mutex in headers. Change to critical section.

// ----------------------------------------------------------------------------
// wxThread implementation
// ----------------------------------------------------------------------------

// wxThreadInternal class
// ----------------------

class wxThreadInternal
{
public:
    wxThreadInternal()
    {
		m_tid = kInvalidID;
		m_state = STATE_NEW;
		m_prio = WXTHREAD_DEFAULT_PRIORITY;
		m_notifyQueueId = kInvalidID;
        m_exitcode = 0;
        m_cancelled = false ;

        // set to true only when the thread starts waiting on m_semSuspend
        m_isPaused = false;

        // defaults for joinable threads
        m_shouldBeJoined = true;
        m_isDetached = false;
    }
    ~wxThreadInternal()
    {
		if ( m_notifyQueueId)
		{
			MPDeleteQueue( m_notifyQueueId);
			m_notifyQueueId = kInvalidID ;
	    }
    }

	// thread function
	static OSStatus    MacThreadStart(void* arg);

    // create a new (suspended) thread (for the given thread object)
    bool Create(wxThread *thread, unsigned int stackSize);

    // thread actions
    // start the thread
    wxThreadError Run();
    // unblock the thread allowing it to run
    void SignalRun() { m_semRun.Post(); }
    // ask the thread to terminate
    void Wait();
    // go to sleep until Resume() is called
    void Pause();
    // resume the thread
    void Resume();

    // accessors
    // priority
    int GetPriority() const { return m_prio; }
    void SetPriority(int prio) ;
    // state
    wxThreadState GetState() const { return m_state; }
    void SetState(wxThreadState state) { m_state = state; }

	// Get the ID of this thread's underlying MP Services task.
	MPTaskID  GetId() const { return m_tid; }

    void SetCancelFlag() { m_cancelled = true; }
    bool WasCancelled() const { return m_cancelled; }
    // exit code
    void SetExitCode(wxThread::ExitCode exitcode) { m_exitcode = exitcode; }
    wxThread::ExitCode GetExitCode() const { return m_exitcode; }

    // the pause flag
    void SetReallyPaused(bool paused) { m_isPaused = paused; }
    bool IsReallyPaused() const { return m_isPaused; }

    // tell the thread that it is a detached one
    void Detach()
    {
        wxCriticalSectionLocker lock(m_csJoinFlag);

        m_shouldBeJoined = false;
        m_isDetached = true;
    }

private:
    // the thread we're associated with
    wxThread *      m_thread;

	MPTaskID        m_tid;	            // thread id
	MPQueueID	    m_notifyQueueId;    // its notification queue

    wxThreadState m_state;              // see wxThreadState enum
    int           m_prio;               // in wxWidgets units: from 0 to 100

    // this flag is set when the thread should terminate
    bool m_cancelled;

    // this flag is set when the thread is blocking on m_semSuspend
    bool m_isPaused;

    // the thread exit code - only used for joinable (!detached) threads and
    // is only valid after the thread termination
    wxThread::ExitCode m_exitcode;

    // many threads may call Wait(), but only one of them should call
    // pthread_join(), so we have to keep track of this
    wxCriticalSection m_csJoinFlag;
    bool m_shouldBeJoined;
    bool m_isDetached;

    // this semaphore is posted by Run() and the threads Entry() is not
    // called before it is done
    wxSemaphore m_semRun;

    // this one is signaled when the thread should resume after having been
    // Pause()d
    wxSemaphore m_semSuspend;
};

OSStatus wxThreadInternal::MacThreadStart(void *parameter)
{
	wxThread* thread = (wxThread*) parameter ;
    wxThreadInternal *pthread = thread->m_internal;

    // add to TLS so that This() will work
	verify_noerr( MPSetTaskStorageValue( gs_tlsForWXThread , (long) thread ) ) ;

    // have to declare this before pthread_cleanup_push() which defines a
    // block!
    bool dontRunAtAll;

    // wait for the semaphore to be posted from Run()
    pthread->m_semRun.Wait();

    // test whether we should run the run at all - may be it was deleted
    // before it started to Run()?
    {
        wxCriticalSectionLocker lock(thread->m_critsect);

        dontRunAtAll = pthread->GetState() == STATE_NEW &&
                       pthread->WasCancelled();
    }

    if ( !dontRunAtAll )
    {
        pthread->m_exitcode = thread->Entry();

        {
            wxCriticalSectionLocker lock(thread->m_critsect);
            pthread->SetState(STATE_EXITED);
        }
    }

    if ( dontRunAtAll )
    {
        if ( pthread->m_isDetached )
            delete thread;

        return -1 ;
    }
    else
    {
        // on mac for the running code the correct thread termination is to
        // return

        // terminate the thread
        thread->Exit(pthread->m_exitcode);

        return (OSStatus) NULL ; // pthread->m_exitcode;
    }
}

bool wxThreadInternal::Create(wxThread *thread, unsigned int stackSize)
{
    wxMacMPThreadsInitVerify() ;
    wxASSERT_MSG( m_state == STATE_NEW && !m_tid,
                    _T("Create()ing thread twice?") );

	OSStatus err = noErr ;
	m_thread = thread;

	if ( m_notifyQueueId == kInvalidID )
    {
		OSStatus err = MPCreateQueue( & m_notifyQueueId);
		if( err)
		{
			wxLogSysError(_("Cant create the thread event queue"));
			return false;
		}
    }

	m_state = STATE_NEW;

	err = MPCreateTask( MacThreadStart,
						(void*) m_thread,
						stackSize,
						m_notifyQueueId,
						&m_exitcode,
						0,
						0,
						&m_tid);

	if ( err)
    {
		wxLogSysError(_("Can't create thread"));
		return false;
    }

	if ( m_prio != WXTHREAD_DEFAULT_PRIORITY )
    {
		SetPriority(m_prio);
    }

	return true;
}

void wxThreadInternal::SetPriority( int priority)
{
	m_prio = priority;

	if ( m_tid)
    {
		// Mac priorities range from 1 to 10,000, with a default of 100.
		// wxWidgets priorities range from 0 to 100 with a default of 50.
		// We can map wxWidgets to Mac priorities easily by assuming
		// the former uses a logarithmic scale.
		const unsigned int macPriority = ( int)( exp( priority / 25.0 * log( 10.0)) + 0.5);

		MPSetTaskWeight( m_tid, macPriority);
    }
}

wxThreadError wxThreadInternal::Run()
{
    wxCHECK_MSG( GetState() == STATE_NEW, wxTHREAD_RUNNING,
                 wxT("thread may only be started once after Create()") );

    SetState(STATE_RUNNING);

    // wake up threads waiting for our start
    SignalRun();

    return wxTHREAD_NO_ERROR;
}

void wxThreadInternal::Wait()
{
   wxCHECK_RET( !m_isDetached, _T("can't wait for a detached thread") );

    // if the thread we're waiting for is waiting for the GUI mutex, we will
    // deadlock so make sure we release it temporarily
    if ( wxThread::IsMain() )
        wxMutexGuiLeave();

    {
        wxCriticalSectionLocker lock(m_csJoinFlag);

        if ( m_shouldBeJoined )
        {
            void * param1;
            void * param2;
            void * rc;

            OSStatus err = MPWaitOnQueue ( m_notifyQueueId,
            			 & param1,
            			 & param2,
            			 & rc,
            			 kDurationForever);
            if ( err)
            {
                wxLogSysError( _( "Cannot wait for thread termination."));
                rc = (void*) -1;
            }

            // actually param1 would be the address of m_exitcode
            // but we don't need this here
            m_exitcode = rc;

            m_shouldBeJoined = false;
        }
    }

    // reacquire GUI mutex
    if ( wxThread::IsMain() )
        wxMutexGuiEnter();
}

void wxThreadInternal::Pause()
{
    // the state is set from the thread which pauses us first, this function
    // is called later so the state should have been already set
    wxCHECK_RET( m_state == STATE_PAUSED,
                 wxT("thread must first be paused with wxThread::Pause().") );

    // wait until the semaphore is Post()ed from Resume()
    m_semSuspend.Wait();
}

void wxThreadInternal::Resume()
{
    wxCHECK_RET( m_state == STATE_PAUSED,
                 wxT("can't resume thread which is not suspended.") );

    // the thread might be not actually paused yet - if there were no call to
    // TestDestroy() since the last call to Pause() for example
    if ( IsReallyPaused() )
    {
        // wake up Pause()
        m_semSuspend.Post();

        // reset the flag
        SetReallyPaused(FALSE);
    }

    SetState(STATE_RUNNING);
}

// static functions
// ----------------

wxThread *wxThread::This()
{
    wxThread* thr = (wxThread*) MPGetTaskStorageValue( gs_tlsForWXThread ) ;
	return thr;
}

bool wxThread::IsMain()
{
	return GetCurrentId() == gs_idMainThread;
}

#ifdef Yield
#undef Yield
#endif

void wxThread::Yield()
{
#if TARGET_API_MAC_OSX
	CFRunLoopRunInMode( kCFRunLoopDefaultMode , 0 , true ) ;
#endif
	MPYield();
}


void wxThread::Sleep(unsigned long milliseconds)
{
	AbsoluteTime wakeup = AddDurationToAbsolute( milliseconds, UpTime());
	MPDelayUntil( & wakeup);
}


int wxThread::GetCPUCount()
{
	return MPProcessors();
}

unsigned long wxThread::GetCurrentId()
{
	return (unsigned long)MPCurrentTaskID();
}


bool wxThread::SetConcurrency(size_t level)
{
    // Cannot be set in MacOS.
	return false;
}


wxThread::wxThread(wxThreadKind kind)
{
	g_numberOfThreads++;
	m_internal = new wxThreadInternal();

	m_isDetached = (kind == wxTHREAD_DETACHED);
}

wxThread::~wxThread()
{
    wxASSERT_MSG( g_numberOfThreads>0 , wxT("More threads deleted than created.") ) ;
    g_numberOfThreads--;

#ifdef __WXDEBUG__
    m_critsect.Enter();

    // check that the thread either exited or couldn't be created
    if ( m_internal->GetState() != STATE_EXITED &&
         m_internal->GetState() != STATE_NEW )
    {
        wxLogDebug(_T("The thread %ld is being destroyed although it is still running! The application may crash."), GetId());
    }

    m_critsect.Leave();
#endif // __WXDEBUG__

    wxDELETE( m_internal ) ;
}


wxThreadError wxThread::Create(unsigned int stackSize)
{
	wxCriticalSectionLocker lock(m_critsect);

    if ( m_isDetached )
    {
        m_internal->Detach() ;
    }
	if ( m_internal->Create(this, stackSize) == false )
    {
        m_internal->SetState(STATE_EXITED);
        return wxTHREAD_NO_RESOURCE;
    }

	return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Run()
{
    wxCriticalSectionLocker lock(m_critsect);

    wxCHECK_MSG( m_internal->GetId(), wxTHREAD_MISC_ERROR,
                 wxT("must call wxThread::Create() first") );

    return m_internal->Run();
}

// -----------------------------------------------------------------------------
// pause/resume
// -----------------------------------------------------------------------------

wxThreadError wxThread::Pause()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 _T("a thread can't pause itself") );

    wxCriticalSectionLocker lock(m_critsect);

    if ( m_internal->GetState() != STATE_RUNNING )
    {
        wxLogDebug(wxT("Can't pause thread which is not running."));

        return wxTHREAD_NOT_RUNNING;
    }

    // just set a flag, the thread will be really paused only during the next
    // call to TestDestroy()
    m_internal->SetState(STATE_PAUSED);

    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Resume()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 _T("a thread can't resume itself") );

    wxCriticalSectionLocker lock(m_critsect);

    wxThreadState state = m_internal->GetState();

    switch ( state )
    {
        case STATE_PAUSED:
            m_internal->Resume();
            return wxTHREAD_NO_ERROR;
        case STATE_EXITED:
            return wxTHREAD_NO_ERROR;

        default:
            wxLogDebug(_T("Attempt to resume a thread which is not paused."));

            return wxTHREAD_MISC_ERROR;
    }
}

// -----------------------------------------------------------------------------
// exiting thread
// -----------------------------------------------------------------------------

wxThread::ExitCode wxThread::Wait()
{
    wxCHECK_MSG( This() != this, (ExitCode)-1,
                 _T("a thread can't wait for itself") );

    wxCHECK_MSG( !m_isDetached, (ExitCode)-1,
                 _T("can't wait for detached thread") );

    m_internal->Wait();

    return m_internal->GetExitCode();
}

wxThreadError wxThread::Delete(ExitCode *rc)
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 _T("a thread can't delete itself") );

    bool isDetached = m_isDetached;

    m_critsect.Enter();
    wxThreadState state = m_internal->GetState();

    // ask the thread to stop
    m_internal->SetCancelFlag();

    m_critsect.Leave();

    switch ( state )
    {
        case STATE_NEW:
            // we need to wake up the thread so that PthreadStart() will
            // terminate - right now it's blocking on run semaphore in
            // PthreadStart()
            m_internal->SignalRun();

            // fall through

        case STATE_EXITED:
            // nothing to do
            break;

        case STATE_PAUSED:
            // resume the thread first
            m_internal->Resume();

            // fall through

        default:
            if ( !isDetached )
            {
                // wait until the thread stops
                m_internal->Wait();

                if ( rc )
                {
                    // return the exit code of the thread
                    *rc = m_internal->GetExitCode();
                }
            }
            //else: can't wait for detached threads
    }

    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Kill()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 _T("a thread can't kill itself") );

    switch ( m_internal->GetState() )
    {
        case STATE_NEW:
        case STATE_EXITED:
            return wxTHREAD_NOT_RUNNING;

        case STATE_PAUSED:
            // resume the thread first
            Resume();

            // fall through

        default:
            OSStatus err = MPTerminateTask( m_internal->GetId() , -1 ) ;
            if ( err )
            {
                wxLogError(_("Failed to terminate a thread."));

                return wxTHREAD_MISC_ERROR;
            }

            if ( m_isDetached )
            {
                delete this ;
            }
            else
            {
                // this should be retrieved by Wait actually
                m_internal->SetExitCode((void*)-1);
            }

            return wxTHREAD_NO_ERROR;
    }
}

void wxThread::Exit(ExitCode status)
{
    wxASSERT_MSG( This() == this,
                  _T("wxThread::Exit() can only be called in the context of the same thread") );

    // don't enter m_critsect before calling OnExit() because the user code
    // might deadlock if, for example, it signals a condition in OnExit() (a
    // common case) while the main thread calls any of functions entering
    // m_critsect on us (almost all of them do)
    OnExit();

    MPTerminateTask( m_internal->GetId() , (long) status) ;

    if ( IsDetached() )
    {
        delete this;
    }
    else // joinable
    {
        // update the status of the joinable thread
        wxCriticalSectionLocker lock(m_critsect);
        m_internal->SetState(STATE_EXITED);
    }
}

// also test whether we were paused
bool wxThread::TestDestroy()
{
    wxASSERT_MSG( This() == this,
                  _T("wxThread::TestDestroy() can only be called in the context of the same thread") );

    m_critsect.Enter();

    if ( m_internal->GetState() == STATE_PAUSED )
    {
        m_internal->SetReallyPaused(TRUE);

        // leave the crit section or the other threads will stop too if they
        // try to call any of (seemingly harmless) IsXXX() functions while we
        // sleep
        m_critsect.Leave();

        m_internal->Pause();
    }
    else
    {
        // thread wasn't requested to pause, nothing to do
        m_critsect.Leave();
    }

    return m_internal->WasCancelled();
}

// -----------------------------------------------------------------------------
// priority setting
// -----------------------------------------------------------------------------

void wxThread::SetPriority(unsigned int prio)
{
    wxCHECK_RET( ((int)WXTHREAD_MIN_PRIORITY <= (int)prio) &&
                 ((int)prio <= (int)WXTHREAD_MAX_PRIORITY),
                 wxT("invalid thread priority") );

    wxCriticalSectionLocker lock(m_critsect);

    switch ( m_internal->GetState() )
    {
        case STATE_RUNNING:
        case STATE_PAUSED:
        case STATE_NEW:
            // thread not yet started, priority will be set when it is
            m_internal->SetPriority(prio);
            break;

        case STATE_EXITED:
        default:
            wxFAIL_MSG(wxT("impossible to set thread priority in this state"));
    }
}

unsigned int wxThread::GetPriority() const
{
    wxCriticalSectionLocker lock((wxCriticalSection &)m_critsect); // const_cast

    return m_internal->GetPriority();
}

unsigned long wxThread::GetId() const
{
    wxCriticalSectionLocker lock((wxCriticalSection &)m_critsect); // const_cast

    return (unsigned long)m_internal->GetId();
}

// -----------------------------------------------------------------------------
// state tests
// -----------------------------------------------------------------------------

bool wxThread::IsRunning() const
{
    wxCriticalSectionLocker lock((wxCriticalSection &)m_critsect);

    return m_internal->GetState() == STATE_RUNNING;
}

bool wxThread::IsAlive() const
{
    wxCriticalSectionLocker lock((wxCriticalSection&)m_critsect);

    switch ( m_internal->GetState() )
    {
        case STATE_RUNNING:
        case STATE_PAUSED:
            return true;

        default:
            return false;
    }
}

bool wxThread::IsPaused() const
{
    wxCriticalSectionLocker lock((wxCriticalSection&)m_critsect);

    return (m_internal->GetState() == STATE_PAUSED);
}

// ----------------------------------------------------------------------------
// Automatic initialization for thread module
// ----------------------------------------------------------------------------

class wxThreadModule : public wxModule
{
public:
	virtual bool OnInit();
	virtual void OnExit();

private:
		DECLARE_DYNAMIC_CLASS(wxThreadModule)
};

IMPLEMENT_DYNAMIC_CLASS(wxThreadModule, wxModule)

bool wxThreadModule::OnInit()
{
	if ( !wxMacMPThreadsInitVerify() )
    {
		return false ;
    }

	verify_noerr( MPAllocateTaskStorageIndex( &gs_tlsForWXThread ) ) ;
	// main thread's This() is NULL
	verify_noerr( MPSetTaskStorageValue( gs_tlsForWXThread , NULL ) ) ;

	gs_idMainThread = wxThread::GetCurrentId() ;

    gs_critsectWaitingForGui = new wxCriticalSection();

    gs_critsectGui = new wxCriticalSection();
    gs_critsectGui->Enter();

	return true;
}

void wxThreadModule::OnExit()
{
    if ( gs_critsectGui )
    {
        gs_critsectGui->Leave();
        delete gs_critsectGui;
        gs_critsectGui = NULL;
    }

    delete gs_critsectWaitingForGui;
    gs_critsectWaitingForGui = NULL;
}

// ----------------------------------------------------------------------------
// GUI Serialization copied from MSW implementation
// ----------------------------------------------------------------------------

void WXDLLIMPEXP_BASE wxMutexGuiEnter()
{
    // this would dead lock everything...
    wxASSERT_MSG( !wxThread::IsMain(),
                  wxT("main thread doesn't want to block in wxMutexGuiEnter()!") );

    // the order in which we enter the critical sections here is crucial!!

    // set the flag telling to the main thread that we want to do some GUI
    {
        wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

        gs_nWaitingForGui++;
    }

    wxWakeUpMainThread();

    // now we may block here because the main thread will soon let us in
    // (during the next iteration of OnIdle())
    gs_critsectGui->Enter();
}

void WXDLLIMPEXP_BASE wxMutexGuiLeave()
{
    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( wxThread::IsMain() )
    {
        gs_bGuiOwnedByMainThread = false;
    }
    else
    {
        // decrement the number of threads waiting for GUI access now
        wxASSERT_MSG( gs_nWaitingForGui > 0,
                      wxT("calling wxMutexGuiLeave() without entering it first?") );

        gs_nWaitingForGui--;

        wxWakeUpMainThread();
    }

    gs_critsectGui->Leave();
}

void WXDLLIMPEXP_BASE wxMutexGuiLeaveOrEnter()
{
    wxASSERT_MSG( wxThread::IsMain(),
                  wxT("only main thread may call wxMutexGuiLeaveOrEnter()!") );

    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( gs_nWaitingForGui == 0 )
    {
        // no threads are waiting for GUI - so we may acquire the lock without
        // any danger (but only if we don't already have it)
        if ( !wxGuiOwnedByMainThread() )
        {
            gs_critsectGui->Enter();

            gs_bGuiOwnedByMainThread = true;
        }
        //else: already have it, nothing to do
    }
    else
    {
        // some threads are waiting, release the GUI lock if we have it
        if ( wxGuiOwnedByMainThread() )
        {
            wxMutexGuiLeave();
        }
        //else: some other worker thread is doing GUI
    }
}

bool WXDLLIMPEXP_BASE wxGuiOwnedByMainThread()
{
    return gs_bGuiOwnedByMainThread;
}

// wake up the main thread
void WXDLLEXPORT wxWakeUpMainThread()
{
	wxMacWakeUp() ;
}

// ----------------------------------------------------------------------------
// include common implementation code
// ----------------------------------------------------------------------------

#include "wx/thrimpl.cpp"

#endif // wxUSE_THREADS
