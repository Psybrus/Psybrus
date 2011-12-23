/**************************************************************************
*
* File:		CsCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSCORE_H__
#define __CSCORE_H__

#include "BcGlobal.h"
#include "BcPath.h"
#include "SysSystem.h"

#include "CsTypes.h"
#include "CsResource.h"
#include "CsResourceRef.h"

#ifdef PSY_SERVER
#include "json/json.h"
#endif

//////////////////////////////////////////////////////////////////////////
/**	\class CsCore
*	\brief Content System Core 
*	
*	Central location to access all game content game or server side.
*	All public methods are thread safe.
*/
class CsCore:
	public BcGlobal< CsCore >,
	public SysSystem
{
public:
	CsCore();
	virtual ~CsCore();
	
public:
	virtual void						open();
	virtual void						update();
	virtual void						close();

public:
	/**
	 * Register a resource.
	 */
	template< typename _Ty >
	void								registerResource();
	
	/**
	 * Create a resource.<br/>
	 * This will create a resource object without a file associated with it.
	 */
	template< typename _Ty >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle );
	template< typename _Ty, typename _A >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA );
	template< typename _Ty, typename _A, typename _B >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB );
	template< typename _Ty, typename _A, typename _B, typename _C >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB, _C ParamC );
	template< typename _Ty, typename _A, typename _B, typename _C, typename _D  >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB, _C ParamC, _D ParamD );

	/**
	 * Request a resource. Will load if it isn't already.
	 * @param Name Name of resource.
	 * @param Handle Handle to be filled in when requested.
	 */
	template< typename _Ty >
	BcBool								requestResource( const BcName& Name, CsResourceRef< _Ty >& Handle );

	/**
	 * Find a resource. Will not load if it isn't already.
	 * @param Name Name of resource.
	 * @param Handle Handle to be filled in when requested.
	 */
	template< typename _Ty >
	BcBool								findResource( const BcName& Name, CsResourceRef< _Ty >& Handle );
	
	/**
	 * Get resource full name.
	 */
	std::string							getResourceFullName( const BcName& Name, const BcName& Type ) const;
	
	
#ifdef PSY_SERVER
	/**
	 * Import a resource using template to derive type.
	 * @param FileName File name of the resource to load.
	 * @param Handle Handle to be filled in upon import.
	 */
	template< typename _Ty >
	BcBool								importResource( const std::string& FileName, CsResourceRef< _Ty >& Handle );
	
	/**
	 * Import a resource from a json object using template to check type.
	 * @param Object Object containing resource.
	 */
	template< typename _Ty >
	BcBool								importObject( const Json::Value& Object, CsResourceRef< _Ty >& Handle, CsDependancyList& DependancyList );

	/**
	 * Get resource property table.
	 * @param Type of resource.
	 * @param PropertyTable Property table to fill in.
	 * @return Success.
	 */
	BcBool								getResourcePropertyTable( const BcName& Type, CsPropertyTable& PropertyTable );

	/**
	 * Add import overlay path.
	 */
	void								addImportOverlayPath( const BcPath& Path );

	/**
	 * Find import path.
	 */
	BcPath							findImportPath( const BcPath& InputPath );

private:
	BcBool								internalImportResource( const std::string& FileName, CsResourceRef<>& Handle, CsDependancyList* pDependancyList );
	BcBool								internalImportObject( const Json::Value& Object, CsResourceRef<>& Handle, CsDependancyList* pDependancyList );
	BcBool								parseJsonFile( const std::string& FileName, Json::Value& Root );

	eEvtReturn							eventOnFileModified( BcU32 EvtID, const FsEventMonitor& Event );
	FsEventMonitor::Delegate			DelegateOnFileModified_;
#endif

protected:
	friend class CsResource;

	void								processCreateResources();
	void								processLoadingResources();
	void								processLoadedResource();
	void								processUnloadingResources();
	
	CsResource*							allocResource( const BcName& Name, const BcName& Type, CsFile* pFile );
	void								destroyResource( CsResource* pResource );

protected:
	/**
	 * Create a file loader.
	 * @param Resource file name.
	 */
	virtual CsFile*						createFileReader( const std::string& FileName );

	/**
	 * Create a file writer.
	 * @param Resource file name.
	 */
	virtual CsFile*						createFileWriter( const std::string& FileName );

public:
	void								internalRegisterResource( const BcName& Type, CsResourceAllocFunc allocFunc, CsResourceFreeFunc freeFunc, CsResourcePropertyTableFunc propertyTableFunc );
	BcBool								internalCreateResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	BcBool								internalRequestResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	BcBool								internalFindResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	
protected:
	struct TResourceFactoryInfo
	{
		CsResourceAllocFunc allocFunc_;
		CsResourceFreeFunc freeFunc_;
		CsResourcePropertyTableFunc propertyTableFunc_;
	};
	
	typedef std::list< CsResource* > TResourceList;
	typedef TResourceList::iterator TResourceListIterator;
	typedef std::list< CsResourceRef<> > TResourceHandleList;
	typedef TResourceHandleList::iterator TResourceHandleListIterator;
	typedef std::map< BcName, TResourceFactoryInfo > TResourceFactoryInfoMap;
	typedef TResourceFactoryInfoMap::iterator TResourceFactoryInfoMapIterator;

	BcMutex								ContainerLock_;
	TResourceFactoryInfoMap				ResourceFactoryInfoMap_;
	TResourceHandleList					CreateResources_;
	TResourceHandleList					LoadingResources_;
	TResourceList						LoadedResources_;
	TResourceList						UnloadingResources_;

#ifdef PSY_SERVER
	typedef std::map< std::string, CsResourceRef<> > TResourceRefMap;
	typedef TResourceRefMap::iterator TResourceRefMapIterator;
	typedef std::map< std::string, CsDependancyList > TDependancyMap;
	typedef TDependancyMap::iterator TDependancyMapIterator;	
	typedef std::list< std::string > TImportList;
	typedef TImportList::iterator TImportListIterator;
	typedef std::list< BcPath > TOverlayList;
	typedef TOverlayList::iterator TOverlayListIterator;

	TResourceRefMap						ResourceImportMap_;
	TDependancyMap						DependancyMap_;
	TImportList							ImportList_;
	TOverlayList						ImportOverlayPaths_;
#endif
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void CsCore::registerResource()
{
	BcAssert( BcIsGameThread() );

	// Register.
	internalRegisterResource( _Ty::StaticGetTypeString(), _Ty::StaticAllocResource, _Ty::StaticFreeResource, _Ty::StaticPropertyTable );

	// Request resource, if there is a failure, reimport if server.
	if( !requestResource( "default", _Ty::Default ) )
	{
#ifdef PSY_SERVER
		importResource( (std::string("EngineContent/default.") + *_Ty::StaticGetTypeString()).c_str(), _Ty::Default );
#endif
	}
}

template< typename _Ty >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetTypeString(), InternalHandle ) )
	{
		Handle->initialise();
		return BcTrue;
	}
	return BcFalse;
}

template< typename _Ty, typename _A >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetTypeString(), InternalHandle ) )
	{
		Handle->initialise( ParamA );
		return BcTrue;
	}
	return BcFalse;
}

template< typename _Ty, typename _A, typename _B >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetTypeString(), InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB );
		return BcTrue;
	}
	return BcFalse;
}

template< typename _Ty, typename _A, typename _B, typename _C >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB, _C ParamC )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetTypeString(), InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC );
		return BcTrue;
	}
	return BcFalse;
}

template< typename _Ty, typename _A, typename _B, typename _C, typename _D >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _A ParamA, _B ParamB, _C ParamC, _D ParamD )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetTypeString(), InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD );
		return BcTrue;
	}
	return BcFalse;
}

template< typename _Ty >
BcForceInline BcBool CsCore::requestResource( const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	return internalRequestResource( Name, _Ty::StaticGetTypeString(), InternalHandle );
}

template< typename _Ty >
BcForceInline BcBool CsCore::findResource( const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	return internalFindResource( Name, _Ty::StaticGetTypeString(), InternalHandle );
}

#ifdef PSY_SERVER
template< typename _Ty >
BcForceInline BcBool CsCore::importResource( const std::string& FileName, CsResourceRef< _Ty >& Handle )
{
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalImportResource( FileName, InternalHandle, NULL ) )
	{
		if( InternalHandle->isTypeOf( _Ty::StaticGetTypeString() ) )
		{
			return BcTrue;
		}
		InternalHandle = NULL;
	}
	return BcFalse;
}

template< typename _Ty >
BcForceInline BcBool CsCore::importObject( const Json::Value& Object, CsResourceRef< _Ty >& Handle, CsDependancyList& DependancyList )
{
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalImportObject( Object, InternalHandle, &DependancyList ) )
	{
		if( InternalHandle->isTypeOf( _Ty::StaticGetTypeString() ) )
		{
			return BcTrue;
		}
		InternalHandle = NULL;
	}
	return BcFalse;
}
#endif

#endif

