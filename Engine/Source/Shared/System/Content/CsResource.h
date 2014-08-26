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

#include <atomic>

#include "Base/BcName.h"
#include "Reflection/ReReflection.h"

#ifdef PSY_SERVER
#include <json/json.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Helper defines.
#define DECLARE_RESOURCE( _Type, _Base )										\
	REFLECTION_DECLARE_DERIVED( _Type, _Base )									\
	public:																	\
	_Type();																	\
	virtual ~_Type();															\
	public:																		

#define DEFINE_RESOURCE( _Type )												\
	REFLECTION_DEFINE_DERIVED( _Type )											\
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
class CsResource:
	public ReObject
{
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( CsResource, ReObject );
public:
	enum
	{
		INIT_STAGE_INITIAL = 0,
		INIT_STAGE_CREATE,
		INIT_STAGE_READY,
		INIT_STAGE_DESTROY
	};

private:
	CsResource( const CsResource& ){}

public:
	CsResource( ReNoInit );
	CsResource();
	virtual ~CsResource();

	/**
	 * Initialise resource. <br/>
	 * Called on construction from thread creating it.
	 */
	virtual void initialise();

	/**
	 * Create resource.<br/>
	 * Called on the content system thread.
	 */
	virtual void create();

	/**
	 * Destroy resource.<br/>
	 * Called on the content system thread.
	 */
	virtual void destroy();

	/**
	 * Set resource index.
	 */
	void setIndex( BcU32 Index );

	/**
	 * Are we ready to use?<br/>
	 */
	BcBool isReady() const;

	/**
	 * Get the init stage.
	 */
	BcU32 getInitStage() const;

	/**
	 * File is ready.
	 */
	virtual void fileReady();

	/**
	 * File chunk is ready.
	 */
	virtual void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

public:
	/**
	 * Get package.
	 */
	CsPackage* getPackage() const;

	/**
	 * Get package name.
	 */
	const BcName& getPackageName() const;
	
	/**
	 * Get index.
	 */
	BcU32 getIndex() const;

protected:
	/**
	 * Get string.
	 */
	const BcChar* getString( BcU32 Offset ) const;

	/**
	 * Markup name.
	 */
	void markupName( BcName& Name ) const;

	/**
	 * Get chunk.
	 */
	void requestChunk( BcU32 Chunk, void* pDataLocation = NULL );

	/**
	 * Get chunk size.
	 */
	BcU32 getChunkSize( BcU32 Chunk );

	/**
	 * Get number of chunks. (See CsFile)
	 */
	BcU32 getNoofChunks() const;

public:
	/**
	 * Mark as ready.
	 */
	void markReady();

	/**
	 * Mark for creation.
	 */
	void markCreate();

	/**
	 * Mark for destruction.
	 */
	void markDestroy();

private:
	friend class CsCore;
	friend class CsPackage;
	friend class CsPackageLoader;

	void onFileReady();
	void onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	//
	BcU32 Index_;
	std::atomic< BcU32 > InitStage_;
};

#endif

