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

#include "Base/BcGlobal.h"
#include "Base/BcPath.h"
#include "System/SysSystem.h"

#include "System/Content/CsTypes.h"
#include "System/Content/CsResource.h"
#include "System/Content/CsResourceRef.h"

#include "System/Content/CsPackage.h"

#ifdef PSY_SERVER
#include "System/Content/CsPackageImporter.h"
#include <json/json.h>
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
	 * Unregister a resource.
	 */
	template< typename _Ty >
	void								unregisterResource();

	/**
	 * Free unreferenced packages.
	 */
	void								freeUnreferencedPackages();
	
	/**
	 * Allocate resource.
	 */
	CsResource*							allocResource( const BcName& Name, const BcName& Type, BcU32 Index, CsPackage* pPackage );

	/*
	 * Destroy resource.
	 */
	void								destroyResource( CsResource* pResource );
	
	/**
	 * Create a resource.<br/>
	 * This will create a resource object without a file associated with it.
	 */
	template< typename _Ty >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle );
	template< typename _Ty, typename _TyA >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA );
	template< typename _Ty, typename _TyA, typename _TyB >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD  >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE  >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF  >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH, typename _TyI >
	BcBool								createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH, _TyI ParamI );
	
	/**
	 * Request a resource. Will load if it isn't already.
	 * @param Package Package to request from.
	 * @param Name Name of resource.
	 * @param Handle Handle to be filled in when requested.
	 */
	template< typename _Ty >
	BcBool								requestResource( const BcName& Package, const BcName& Name, CsResourceRef< _Ty >& Handle );

	/**
	 * Get number of resources.
	 */
	BcU32								getNoofResources();

	/**
	 * Get resource by index.
	 */
	CsResourceRef<>						getResource( BcU32 Idx );

	/**
	 * Get resource. PLACEHOLDER UNTIL COMPONENTS ARE PACKED PROPERLY.
	 * @param pFullName Full name in package.resource.type format.
	 */
	CsResourceRef<>						getResource( const BcChar* pFullName );

	/**
	 * Request package.
	 * @param Package Name of package.
	 * @return Success in requesting the package.
	 */
	CsPackage*							requestPackage( const BcName& Package );

	/**
	 * Find package.
	 * @param Package Name of package.
	 * @return Found package.
	 */
	CsPackage*							findPackage( const BcName& Package );
	
	/**
	 * Get package import path.
	 */
	BcPath								getPackageImportPath( const BcName& Package );

	/**
	 * Get package packed path.
	 */
	BcPath								getPackagePackedPath( const BcName& Package );

protected:
	friend class CsResource;

	void								processCreateResources();
	void								processLoadingResources();
	void								processLoadedResource();
	void								processUnloadingResources();
	
public:
	void								internalRegisterResource( const BcName& Type, CsResourceAllocFunc allocFunc, CsResourceFreeFunc freeFunc );
	void								internalUnRegisterResource( const BcName& Type );
	BcBool								internalCreateResource( const BcName& Name, const BcName& Type, BcU32 Index, CsPackage* pPackage, CsResourceRef<>& Handle );
	BcBool								internalRequestResource( const BcName& Package, const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	BcBool								internalFindResource( const BcName& Package, const BcName& Name, const BcName& Type, CsResourceRef<>& Handle );
	
protected:
	struct TResourceFactoryInfo
	{
		CsResourceAllocFunc allocFunc_;
		CsResourceFreeFunc freeFunc_;
	};
	
	typedef std::vector< CsResource* > TResourceList;
	typedef TResourceList::iterator TResourceListIterator;
	typedef std::vector< CsResourceRef<> > TResourceHandleList;
	typedef TResourceHandleList::iterator TResourceHandleListIterator;
	typedef std::map< BcName, TResourceFactoryInfo > TResourceFactoryInfoMap;
	typedef TResourceFactoryInfoMap::iterator TResourceFactoryInfoMapIterator;

	typedef std::list< CsPackage* > TPackageList;
	typedef TPackageList::iterator TPackageListIterator;

	BcMutex								ContainerLock_;
	TResourceFactoryInfoMap				ResourceFactoryInfoMap_;
	TResourceHandleList					CreateResources_;
	TResourceHandleList					LoadingResources_;
	TResourceList						LoadedResources_;
	TResourceList						UnloadingResources_;
	
	TPackageList						PackageList_;
	TPackageList						UnreferencedPackageList_;

	BcBool								IsCollectingGarbage_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void CsCore::registerResource()
{
	BcAssert( BcIsGameThread() );
	internalRegisterResource( _Ty::StaticGetType(), _Ty::StaticAllocResource, _Ty::StaticFreeResource );
}

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void CsCore::unregisterResource()
{
	BcAssert( BcIsGameThread() );
	internalUnRegisterResource( _Ty::StaticGetType() );
}

template< typename _Ty >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise();
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG, ParamH );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH, typename _TyI >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsResourceRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH, _TyI ParamI )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetType(), BcErrorCode, NULL, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG, ParamH, ParamI );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty >
BcForceInline BcBool CsCore::requestResource( const BcName& Package, const BcName& Name, CsResourceRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Handle ) );
	return internalRequestResource( Package, Name, _Ty::StaticGetType(), InternalHandle );
}

#endif

