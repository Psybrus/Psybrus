/**************************************************************************
*
* File:		CsPackage.h
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSPACKAGE_H__
#define __CSPACKAGE_H__

#include "System/Content/CsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class CsPackageLoader;

//////////////////////////////////////////////////////////////////////////
// CsPackage
class CsPackage
{
private:
	CsPackage( const CsPackage& ){}

public:
	CsPackage( const BcName& Name );
	~CsPackage();

	/**
	 * Are we ready?
	 */
	BcBool							isReady() const;

	/**
	 * Are we loaded?
	 */
	BcBool							isLoaded() const;

	/**
	 * Do we have any unreferenced resources?
	 */
	BcBool							hasUnreferencedResources() const;

	/**
	 * Have we got any valid resources?
	 */
	BcBool							haveAnyValidResources() const;

	/**
	 * Release unreferenced resources.
	 */
	void							releaseUnreferencedResources();
	
	/**
	 * Add resource to package.
	 */
	void							addResource( CsResource* pResource );

	/**
	* Get resource.
	*/
	CsResource*						getResource( BcU32 ResourceIdx );

	/**
	* Load a package cross ref.
	*/
	CsPackage*						loadPackageCrossRef( BcU32 ID );

	/**
	* Get package cross ref.
	*/
	ReObjectRef< CsResource >					getPackageCrossRef( BcU32 ID );

	/**
	* Acquire.
	*/
	void							acquire();

	/**
	* Release
	*/
	void							release();

public:
	/**
	 * Get name.
	 */
	const BcName&					getName() const;

public:
	/**
	 * Get string.
	 * @param Offset Offset of string in table.
	 * @return String, or NULL pointer if invalid offset.
	 */
	const BcChar*					getString( BcU32 Offset ) const;
	BcU32							getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx ) const;
	BcU32							getNoofChunks( BcU32 ResourceIdx ) const;	
	void							markupName( BcName& Name ) const;
	BcBool							requestChunk( BcU32 ResourceIdx, BcU32 ResourceChunkIdx, void* pDataLocation = NULL );
	
private:
	BcName							Name_;
	BcAtomicU32						RefCount_;

	// Loader we use.
	CsPackageLoader*				pLoader_;

	// Resources.
	typedef std::vector< ReObjectRef< CsResource > > TResourceHandleList;
	TResourceHandleList				Resources_;
	
};

#endif
