/**************************************************************************
*
* File:		CsResource.h
* Author:	Neil Richardson
* Ver/Date:	7/03/11
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSRESOURCE_H__
#define __CSRESOURCE_H__

#include "System/Content/CsTypes.h"

#include "System/Content/CsResourceRef.h"

#include "Base/BcAtomic.h"
#include "Base/BcAtomicMutex.h"
#include "Base/BcScopedLock.h"
#include "Base/BcName.h"
#include "Base/BcReflection.h"
#include "Base/BcReflectionSerialise.h"

#ifdef PSY_SERVER
#include <json/json.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Helper defines.
#define DECLARE_RESOURCE( _Base, _Type )										\
	BCREFLECTION_DECLARE_DERIVED( _Base, _Type )								\
	protected:																	\
	_Type();																	\
	virtual ~_Type();															\
	public:																		

#define DEFINE_RESOURCE( _Type )												\
	BCREFLECTION_DEFINE_DERIVED( _Type )										\
	_Type::_Type()																\
	{																			\
	}																			\
																				\
	_Type::~_Type()																\
	{}																			
																				
//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsPackage;
class CsResource;
class CsCore;

//////////////////////////////////////////////////////////////////////////
// CsResource
class CsResource
{
	BCREFLECTION_DECLARE_BASE( CsResource );
	
private:
	CsResource( const CsResource& ){}

public:
	CsResource();
	virtual ~CsResource();

	/**
	 * Pre-initialise. Setup everything before derived initialisation.
	 */
	void							preInitialise( const BcName& Name, BcU32 Index, CsPackage* pPackage );

#ifdef PSY_SERVER
	/**
	 * Import resource.
	 */
	virtual BcBool					import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif

	/**
	 * Initialise resource. <br/>
	 * Called on construction from thread creating it.
	 */
	virtual void					initialise();

	/**
	 * Create resource.<br/>
	 * Called on the content system thread.
	 */
	virtual void					create();

	/**
	 * Destroy resource.<br/>
	 * Called on the content system thread.
	 */
	virtual void					destroy();

	/**
	 * Are we ready to use?<br/>
	 */
	BcBool							isReady();

	/**
	 * File is ready.
	 */
	virtual void					fileReady();

	/**
	 * File chunk is ready.
	 */
	virtual void					fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

public:
	/**
	 * Acquire resource.
	 * TODO: Deprecate.
	 */
	void							acquire();

	/**
	 * Release resource.
	 * TODO: Deprecate.
	 */
	void							release();

	/**
	 * Get ref count.
	 */
	BcU32							refCount() const;

	/**
	 * Get package.
	 */
	CsPackage*						getPackage();

	/**
	 * Get package name.
	 */
	const BcName&					getPackageName() const;
	
	/**
	 * Get name.
	 */
	const BcName&					getName() const;

	/**
	 * Get index.
	 */
	BcU32							getIndex() const;

	/**
	 * Serialise properties.
	 */
	virtual void					serialiseProperties();

protected:
	/**
	 * Get string.
	 */
	const BcChar*					getString( BcU32 Offset );

	/**
	 * Get chunk.
	 */
	void							requestChunk( BcU32 Chunk, void* pDataLocation = NULL );

	/**
	 * Get chunk size.
	 */
	BcU32							getChunkSize( BcU32 Chunk );

	/**
	 * Get number of chunks. (See CsFile)
	 */
	BcU32							getNoofChunks() const;

	/**
	 * Mark as ready.
	 */
	void							markReady();

private:
	friend class CsCore;
	friend class CsPackageLoader;

	void							onFileReady();
	void							onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	//
	BcName							Name_;
	BcU32							Index_;
	CsPackage*						pPackage_;
	
	//
	BcAtomicU32						RefCount_;	// TODO: Deprecate.
	BcAtomicU32						IsReady_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
#include "System/Content/CsResourceRef.inl"

#endif

