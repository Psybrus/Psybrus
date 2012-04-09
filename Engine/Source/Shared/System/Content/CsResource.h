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

#ifdef PSY_SERVER
#include <json/json.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Helper defines.
#define BASE_DECLARE_RESOURCE( _Type )											\
	public:																		\
	static const BcName& StaticGetType();										\
	static BcHash StaticGetTypeHash();											\
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
	_Type( const BcName& Name, BcU32 Index, CsPackage* pPackage );				\
	virtual ~_Type();															\
	public:																		\
	static CsResource* StaticAllocResource( const BcName& Name,					\
	                                        BcU32 Index,						\
											CsPackage* pPackage );				\
	static void StaticFreeResource( CsResource* pResource );					\
	static BcU32 StaticGetClassSize();

#define DEFINE_RESOURCE( _Type )												\
	BASE_DEFINE_RESOURCE( _Type )												\
	_Type::_Type( const BcName& Name, BcU32 Index, CsPackage* pPackage ):		\
		Super( Name, Index, pPackage )											\
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
	                                        BcU32 Index,						\
	                                        CsPackage* pPackage )				\
	{																			\
		return new _Type( Name, Index, pPackage );								\
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
class CsPackage;
class CsResource;
class CsCore;

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResource*( *CsResourceAllocFunc )( const BcName&, BcU32, CsPackage* );
typedef void( *CsResourceFreeFunc )( CsResource* );

//////////////////////////////////////////////////////////////////////////
// CsResource
class CsResource
{
public:
	DECLARE_CSRESOURCE;
	
private:
	CsResource( const CsResource& ){}

public:
	CsResource( const BcName& Name, BcU32 Index, CsPackage* pPackage );
	virtual ~CsResource();

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
	virtual void					fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

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
	BcAtomicU32						RefCount_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
#include "System/Content/CsResourceRef.inl"

#endif

