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
	 * Get resource type.
	 */
	BcName								getResourceType( BcU32 Idx ) const;

	/**
	 * Get noof resource types.
	 */
	BcU32								getNoofResourceTypes() const;
	
	/**
	 * Allocate resource.
	 */
	CsResource*							allocResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage );

	/*
	 * Destroy resource.
	 */
	void								destroyResource( CsResource* pResource );
	
	/*
	* Get a resource by its Unique Id
	*/
	ReObjectRef< CsResource >						getResourceByUniqueId(BcU32 UId);

	/**
	 * Create a resource.<br/>
	 * This will create a resource object without a file associated with it.
	 */
	template< typename _Ty >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle );
	template< typename _Ty, typename _TyA >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA );
	template< typename _Ty, typename _TyA, typename _TyB >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD  >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE  >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF  >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH );
	template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH, typename _TyI >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH, _TyI ParamI );
	
	/**
	 * Request a resource. Will load if it isn't already.
	 * @param Package Package to request from.
	 * @param Name Name of resource.
	 * @param Handle Handle to be filled in when requested.
	 */
	template< typename _Ty >
	BcBool								requestResource( const BcName& Package, const BcName& Name, ReObjectRef< _Ty >& Handle );

	/**
	 * Get number of resources.
	 */
	BcU32								getNoofResources();

	/**
	 * Get resource by index.
	 */
	ReObjectRef< CsResource >			getResource( BcU32 Idx );

	/**
	 * Get resource. PLACEHOLDER UNTIL COMPONENTS ARE PACKED PROPERLY.
	 * @param pFullName Full name in package.resource.type format.
	 */
	ReObjectRef< CsResource >			getResource( const BcChar* pFullName );

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
	 * Request a callback when a package is ready.
	 * @params Package Package to register for.
	 * @params Callback Delegate to call.
	 * @param ID ID to pass into delegate.
	 */
	void								requestPackageReadyCallback( const BcName& Package, const CsPackageReadyCallback& Callback, BcU32 ID );
	
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
	void								processCallbacks();
	
public:
	void								internalRegisterResource( const ReClass* pClass );
	void								internalUnRegisterResource( const ReClass* Class );
	BcBool								internalCreateResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage, ReObjectRef< CsResource >& Handle );
	BcBool								internalRequestResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle );
	BcBool								internalFindResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle );
	
protected:
	struct TResourceFactoryInfo // TODO: Deprecate. We don't need this much longer with having centralised reflection support.
	{
		const ReClass* pClass_;
	};
	
	typedef std::vector< CsResource* > TResourceList;
	typedef TResourceList::iterator TResourceListIterator;
	typedef std::vector< ReObjectRef< CsResource > > TResourceHandleList;
	typedef TResourceHandleList::iterator TResourceHandleListIterator;
	typedef std::map< const ReClass*, TResourceFactoryInfo > TResourceFactoryInfoMap;
	typedef TResourceFactoryInfoMap::iterator TResourceFactoryInfoMapIterator;

	struct TPackageReadyCallback
	{
		BcName Package_;
		CsPackageReadyCallback Callback_;
		BcU32 ID_;
	};

	typedef std::list< TPackageReadyCallback > TPackageReadyCallbackList;
	typedef TPackageReadyCallbackList::iterator TPackageReadyCallbackListIterator;

	typedef std::list< CsPackage* > TPackageList;
	typedef TPackageList::iterator TPackageListIterator;

	BcMutex								ContainerLock_;
	TResourceFactoryInfoMap				ResourceFactoryInfoMap_;
	TResourceHandleList					PrecreateResources_;
	TResourceHandleList					CreateResources_;
	TResourceHandleList					LoadingResources_;
	TResourceList						LoadedResources_;
	TResourceList						UnloadingResources_;
	
	TPackageList						PackageList_;
	TPackageList						UnreferencedPackageList_;

	TPackageReadyCallbackList			PackageReadyCallbackList_;

	BcBool								IsCollectingGarbage_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void CsCore::registerResource()
{
	BcAssert( BcIsGameThread() );
	_Ty::StaticRegisterClass();
	internalRegisterResource( _Ty::StaticGetClass() );
}

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void CsCore::unregisterResource()
{
	BcAssert( BcIsGameThread() );
	internalUnRegisterResource( _Ty::StaticGetClass() );
}

template< typename _Ty >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise();
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG, ParamH );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty, typename _TyA, typename _TyB, typename _TyC, typename _TyD, typename _TyE, typename _TyF, typename _TyG, typename _TyH, typename _TyI >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _TyA ParamA, _TyB ParamB, _TyC ParamC, _TyD ParamD, _TyE ParamE, _TyF ParamF, _TyG ParamG, _TyH ParamH, _TyI ParamI )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( ParamA, ParamB, ParamC, ParamD, ParamE, ParamF, ParamG, ParamH, ParamI );
		return BcTrue;
	}
	BcVerifyMsg( BcFalse, "CsCore::createResource Failed!" );
	return BcFalse;
}

template< typename _Ty >
BcForceInline BcBool CsCore::requestResource( const BcName& Package, const BcName& Name, ReObjectRef< _Ty >& Handle )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	return internalRequestResource( Package, Name, _Ty::StaticGetClass(), InternalHandle );
}

#endif

