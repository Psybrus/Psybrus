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
#include "System/Content/CsFile.h"
#include "System/Content/CsProperty.h"

#include "System/Content/CsResourceRef.h"

#include "Base/BcAtomic.h"
#include "Base/BcAtomicMutex.h"
#include "Base/BcScopedLock.h"
#include "Base/BcName.h"

#ifdef PSY_SERVER
#include <json/json.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Helper defines.
#define BASE_DECLARE_RESOURCE( _Type )											\
	public:																		\
	static CsResourceRef< class _Type > Default;								\
	static const BcName& StaticGetType();										\
	static BcHash StaticGetTypeHash();											\
	static void StaticPropertyTable( CsPropertyTable& PropertyTable );			\
	virtual const BcName& getType();											\
	virtual BcHash getTypeHash();												\
	virtual BcBool isType( const BcName& Type );								\
	virtual BcBool isTypeOf( const BcName& Type );								\
	template < class _Ty >														\
	BcForceInline BcBool isTypeOf()												\
	{																			\
		return this ? isTypeOf( _Ty::StaticGetType() ) : BcFalse;				\
	}

#define BASE_DEFINE_RESOURCE( _Type )											\
	CsResourceRef< class _Type > _Type::Default;								\
	const BcName& _Type::StaticGetType()										\
	{																			\
		static BcName TypeString( #_Type );										\
		return TypeString;														\
	}																			\
																				\
	BcHash _Type::StaticGetTypeHash()											\
	{																			\
		return BcHash( #_Type );												\
	}																			\
																				\
	const BcName& _Type::getType()												\
	{																			\
		return _Type::StaticGetType();											\
	}																			\

#define DECLARE_CSRESOURCE														\
	BASE_DECLARE_RESOURCE( CsResource )											\

#define DEFINE_CSRESOURCE														\
	BASE_DEFINE_RESOURCE( CsResource )											\
	BcHash CsResource::getTypeHash()											\
	{																			\
		return CsResource::StaticGetTypeHash();									\
	}																			\
																				\
	BcBool CsResource::isType( const BcName& Type )								\
	{																			\
		return CsResource::StaticGetType() == Type;								\
	}																			\
																				\
	BcBool CsResource::isTypeOf( const BcName& Type )							\
	{																			\
		return CsResource::StaticGetType() == Type;								\
	}																			\



#define DECLARE_RESOURCE( _Base, _Type )										\
	BASE_DECLARE_RESOURCE( _Type )												\
	typedef _Base Super;														\
	protected:																	\
	_Type( const BcName& Name, CsFile* pFile );									\
	virtual ~_Type();															\
	public:																		\
	static CsResource* StaticAllocResource( const BcName& Name,					\
											CsFile* pFile );					\
	static void StaticFreeResource( CsResource* pResource );					\
	static BcU32 StaticGetClassSize();

#define DEFINE_RESOURCE( _Type )												\
	BASE_DEFINE_RESOURCE( _Type )												\
	_Type::_Type( const BcName& Name, CsFile* pFile ):							\
		Super( Name, pFile )													\
	{																			\
	}																			\
																				\
	_Type::~_Type()																\
	{}																			\
	BcHash _Type::getTypeHash()													\
	{																			\
		return _Type::StaticGetTypeHash();										\
	}																			\
																				\
	BcBool _Type::isType( const BcName& Type )									\
	{																			\
		return  _Type::StaticGetType() == Type;									\
	}																			\
																				\
	BcBool _Type::isTypeOf( const BcName& Type )								\
	{																			\
		return _Type::StaticGetType() == Type || Super::isTypeOf( Type );		\
	}																			\
																				\
	CsResource* _Type::StaticAllocResource( const BcName& Name,					\
	                                        CsFile* pFile )						\
	{																			\
		return new _Type( Name, pFile );										\
	}																			\
																				\
	void _Type::StaticFreeResource( CsResource* pResource )						\
	{																			\
		delete pResource;														\
	}																			\
																				\
	BcU32 _Type::StaticGetClassSize()											\
	{																			\
		return sizeof( _Type );													\
	}

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsResource;
class CsCore;

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResource*( *CsResourceAllocFunc )( const BcName&, CsFile* );
typedef void( *CsResourceFreeFunc )( CsResource* );
typedef void( *CsResourcePropertyTableFunc )( CsPropertyTable& );

//////////////////////////////////////////////////////////////////////////
// CsResource
class CsResource
{
public:
	DECLARE_CSRESOURCE;
	
private:
	CsResource( const CsResource& ){}

public:
	CsResource( const BcName& Name, CsFile* pFile );
	virtual ~CsResource();

#ifdef PSY_SERVER
	/**
	 * Import resource.
	 */
	virtual BcBool					import( const Json::Value& Object, CsDependancyList& DependancyList );
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
	 * Should return true *only* when a resource is fully ready to use.
	 */
	virtual BcBool					isReady();

	/**
	 * File is ready.
	 */
	virtual void					fileReady();

	/**
	 * File chunk is ready.
	 */
	virtual void					fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

public:
	/**
	 * Acquire resource.
	 */
	void							acquire();

	/**
	 * Release resource.
	 */
	void							release();

	/**
	 * Get name.
	 */
	const BcName&					getName() const;


protected:
	/**
	 * Get string. (See CsFile)
	 */
	const BcChar*					getString( BcU32 Offset );

	/**
	 * Get chunk. (See CsFile)
	 */
	void							getChunk( BcU32 Chunk, BcBool TriggerLoad = BcTrue );

	/**
	 * Get number of chunks. (See CsFile)
	 */
	BcU32							getNoofChunks() const;

private:
	friend class CsCore;

	void							delegateFileReady( CsFile* pFile );
	void							delegateFileChunkReady( CsFile* pFile, BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

#if PSY_SERVER
protected:
#else
private:
#endif
	CsFile*							pFile_;

private:
	BcName							Name_;
	BcAtomicU32						RefCount_;
	BcAtomicMutex					Lock_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
#include "System/Content/CsResourceRef.inl"

#endif

