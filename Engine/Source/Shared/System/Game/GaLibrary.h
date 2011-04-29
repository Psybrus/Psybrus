/**************************************************************************
*
* File:		GaLibrary.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaLibrary_H__
#define __GaLibrary_H__

#include "GaCore.h"
#include "CsCore.h"
#include "CsResourceRef.h"

#include "gmMachine.h"
#include "gmThread.h"
#include "gmVariable.h"
#include "gmHelpers.h"

//////////////////////////////////////////////////////////////////////////
// GaLibraryResource
template < class _Ty >
class GaLibraryResource
{
public:
	static gmType GM_TYPE;
	
public:
	static gmUserObject* GM_CDECL AllocUserObject( gmMachine* a_machine, CsResourceRef< _Ty > Resource )
	{
		gmUserObject* a_object = NULL;
		
		if( Resource.isValid() )
		{
			_Ty* pResource = Resource;
			a_object = a_machine->AllocUserObject( pResource, GM_TYPE );
			
			// Acquire resource.
			pResource->acquire();
		}
		
		return a_object;
	}
	
	static bool GM_CDECL Trace( gmMachine * a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
	{
		a_workDone++;	
		return true;
	}
	
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object )
	{
		_Ty* pResource = (_Ty*)a_object->m_user;
		
		// Release resource.
		pResource->release();
	}
	
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
	{
		_Ty* pResource = (_Ty*)a_object->m_user;
		
		BcSPrintf( a_buffer, "<%s Object \"%s\" @ %p>", pResource->getTypeString().c_str(), pResource->getName().c_str(), pResource );
	}
	
	static int GM_CDECL Request( gmThread* a_thread )
	{
		GM_CHECK_NUM_PARAMS( 1 );
		GM_CHECK_STRING_PARAM( pResourceName, 0 );
		
		CsResourceRef< _Ty > Resource;
		
		if( CsCore::pImpl()->requestResource( pResourceName, Resource ) )
		{
			// Alloc user object if we can.
			gmUserObject* a_object = AllocUserObject( a_thread->GetMachine(), Resource );
			
			// If it's valid push onto stack and add resource block.
			if( a_object != NULL )
			{
				a_thread->PushUser( a_object );
				
				return GaCore::pImpl()->addResourceBlock( CsResourceRef<>( Resource ), a_object, a_thread ) ? GM_SYS_BLOCK : GM_OK;
			}
		}
		else
		{
			BcPrintf( "GaCore: Request %s.%s failed!\n", pResourceName, _Ty::StaticGetTypeString().c_str() );
		}
		
		return GM_OK;
	}
	
	static void GM_CDECL CreateType( gmMachine* a_machine )
	{
		GM_TYPE = a_machine->CreateUserType( _Ty::StaticGetTypeString().c_str() );
		a_machine->RegisterUserCallbacks( GM_TYPE,
										 &GaLibraryResource< _Ty >::Trace,
										 &GaLibraryResource< _Ty >::Destruct,
										 &GaLibraryResource< _Ty >::AsString ); 
	}
	
};

//////////////////////////////////////////////////////////////////////////
// GaLibraryResource statics
template < class _Ty >
gmType GaLibraryResource< _Ty >::GM_TYPE = GM_NULL;


#endif


