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
#define DECLARE_RESOURCE( _Base, _Type )										\
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
	REFLECTION_DECLARE_DERIVED( CsResource, ReObject );
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
	BcBool							isReady() const;

	/**
	 * Get the init stage.
	 */
	BcU32							getInitStage() const;

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

	/**
	 * Get unique identifier for resource
	 */
	inline const BcU32				getUniqueId() const
	{
//		return (BcU32)this;
		return UniqueId_;
	}
protected:
	/**
	 * Get string.
	 */
	const BcChar*					getString( BcU32 Offset ) const;

	/**
	 * Markup name.
	 */
	void							markupName( BcName& Name ) const;

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

	/**
	 * Mark for creation.
	 */
	void							markCreate();

	/**
	 * Mark for destruction.
	 */
	void							markDestroy();

private:
	friend class CsCore;
	friend class CsPackage;
	friend class CsPackageLoader;

	void							onFileReady();
	void							onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	//
	BcName							Name_;
	BcU32							Index_;
	CsPackage*						pPackage_;
	std::atomic< BcU32 >			InitStage_;

	BcU32							UniqueId_;

	static std::atomic< BcU32 >		UniqueIdCounter_;
};

#endif

