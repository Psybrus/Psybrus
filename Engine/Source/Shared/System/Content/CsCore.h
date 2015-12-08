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

#ifdef PSY_IMPORT_PIPELINE
#include "System/Content/CsPackageImporter.h"
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
	* Get a resource by its Unique Id
	*/
	ReObjectRef< CsResource >			getResourceByUniqueId(BcU32 UId);

	/**
	 * Create a resource.<br/>
	 * This will create a resource object without a file associated with it.
	 */
	template< typename _Ty, typename... _ParamT >
	BcBool								createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _ParamT... Params );
	
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
	size_t								getNoofResources();

	/**
	 * Get resource by index.
	 */
	ReObjectRef< CsResource >			getResource( size_t Idx );

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
	
protected:
	friend class CsResource;

	void								processResources();
	void								processCallbacks();
	
public:
	void								internalAddResource( CsResource* Resource );
	void								internalAddResourceForProcessing( CsResource* Resource );
	BcBool								internalCreateResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage, ReObjectRef< CsResource >& Handle );
	BcBool								internalRequestResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle );
	BcBool								internalFindResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle );
	
protected:
	typedef std::vector< CsResource* > TResourceList;
	typedef TResourceList::iterator TResourceListIterator;

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

	std::recursive_mutex				ContainerLock_;
	TResourceList						PreprocessResources_;
	TResourceList						ProcessingResources_;
	TResourceList						Resources_;
	
	TPackageList						PackageList_;
	TPackageList						UnloadingPackageList_;
	TPackageList						UnreferencedPackageList_;

	TPackageReadyCallbackList			PackageReadyCallbackList_;

	BcBool								IsCollectingGarbage_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty, typename... _ParamT >
BcForceInline BcBool CsCore::createResource( const BcName& Name, CsPackage* pPackage, ReObjectRef< _Ty >& Handle, _ParamT... Params )
{
	BcAssert( BcIsGameThread() );
	ReObjectRef< CsResource >& InternalHandle = *( reinterpret_cast< ReObjectRef< CsResource >* >( &Handle ) );
	if( internalCreateResource( Name, _Ty::StaticGetClass(), BcErrorCode, pPackage, InternalHandle ) )
	{
		Handle->initialise( Params... );
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

