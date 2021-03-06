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
class CsPackage:
	public ReObject
{
public:
	REFLECTION_DECLARE_DERIVED( CsPackage, ReObject );

private:
	CsPackage( const CsPackage& ){}

public:
	CsPackage();
	CsPackage( const BcName& Name, const BcPath& Filename );
	~CsPackage();

	/**
	 * Are we ready?
	 */
	BcBool							isReady() const;

	/**
	 * Are we loading?
	 */
	BcBool							isLoading() const;

	/**
	 * Are we loaded?
	 */
	BcBool							isLoaded() const;

	/**
	 * Are we unloading?
	 */
	BcBool							isUnloading() const;

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
	CsPackage*						getCrossRefPackage( BcU32 ID );

	/**
	* Get package cross ref.
	*/
	CsResource*						getCrossRefResource( BcU32 ID );

	/**
	* Acquire.
	* @return Number of references.
	*/
	BcU32							acquire();

	/**
	* Release
	* @return Numver of references.
	*/
	BcU32							release();

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
	std::atomic< BcU32 >			RefCount_;

	// Loader we use.
	CsPackageLoader*				pLoader_;

	// Resources.
	typedef std::vector< ReObjectRef< CsResource > > TResourceHandleList;
	TResourceHandleList				Resources_;
	
};

#endif
