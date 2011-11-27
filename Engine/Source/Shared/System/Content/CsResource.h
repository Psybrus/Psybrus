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

#include "CsTypes.h"
#include "CsFile.h"
#include "CsProperty.h"

#include "BcAtomic.h"
#include "BcAtomicMutex.h"
#include "BcScopedLock.h"

#ifdef PSY_SERVER
#include "json/json.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Helper defines.

#define BASE_DECLARE_RESOURCE( _Type )											\
	public:																		\
	static const std::string& StaticGetTypeString();							\
	static BcHash StaticGetTypeHash();											\
	static void StaticPropertyTable( CsPropertyTable& PropertyTable );			\
	virtual const std::string& getTypeString();									\
	virtual BcHash getTypeHash();												\
	virtual BcBool isType( const std::string& Type );							\
	virtual BcBool isTypeOf( const std::string& Type );							

#define BASE_DEFINE_RESOURCE( _Type )											\
	const std::string& _Type::StaticGetTypeString()								\
	{																			\
		static std::string TypeString( #_Type );								\
		return TypeString;														\
	}																			\
																				\
	BcHash _Type::StaticGetTypeHash()											\
	{																			\
		return BcHash( #_Type );												\
	}																			\
																				\
	const std::string& _Type::getTypeString()									\
	{																			\
		return _Type::StaticGetTypeString();									\
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
	BcBool CsResource::isType( const std::string& Type )						\
	{																			\
		return CsResource::StaticGetTypeString() == Type;						\
	}																			\
																				\
	BcBool CsResource::isTypeOf( const std::string& Type )						\
	{																			\
		return CsResource::StaticGetTypeString() == Type;						\
	}																			\

#define DECLARE_RESOURCE( _Base, _Type )										\
	BASE_DECLARE_RESOURCE( _Type )												\
	typedef _Base Super;														\
	protected:																	\
	_Type( const std::string& Name, CsFile* pFile );							\
	virtual ~_Type();															\
	public:																		\
	static CsResource* StaticAllocResource( const std::string& Name,			\
	CsFile* pFile );															\
	static void StaticFreeResource( CsResource* pResource );					\


#define DEFINE_RESOURCE( _Type )												\
	BASE_DEFINE_RESOURCE( _Type )												\
	_Type::_Type( const std::string& Name, CsFile* pFile ):						\
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
	BcBool _Type::isType( const std::string& Type )								\
	{																			\
		return  _Type::StaticGetTypeString() == Type;							\
	}																			\
																				\
	BcBool _Type::isTypeOf( const std::string& Type )							\
	{																			\
		return _Type::StaticGetTypeString() == Type || Super::isTypeOf( Type );	\
	}																			\
																				\
	CsResource* _Type::StaticAllocResource( const std::string& Name,			\
	                                        CsFile* pFile )						\
	{																			\
		return new _Type( Name, pFile );										\
	}																			\
																				\
	void _Type::StaticFreeResource( CsResource* pResource )						\
	{																			\
		delete pResource;														\
	}																			

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
enum CsResourceStage
{
	csRS_PENDING_CREATE = 0,
	csRS_CREATE,
	csRS_LOADING,
	csRS_LOADED,
	csRS_DESTROY,
	csRS_KILL,
	
	csRS_MAX
};

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsResource;
class CsCore;

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResource*( *CsResourceAllocFunc )( const std::string&, CsFile* );
typedef void( *CsResourceFreeFunc )( CsResource* );
typedef void( *CsResourcePropertyTableFunc )( CsPropertyTable& );

//////////////////////////////////////////////////////////////////////////
// CsResource
class CsResource
{
public:
	DECLARE_CSRESOURCE;
	
private: // non-copyable.
	BcForceInline CsResource( const CsResource& ){};
	BcForceInline CsResource& operator = ( const CsResource& ){};

public:
	CsResource( const std::string& Name, CsFile* pFile );
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
	const std::string&				getName() const;
	
	
protected:
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
	
	CsResourceStage					process();
	void							delegateFileReady( CsFile* pFile );
	void							delegateFileChunkReady( CsFile* pFile, BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

#if PSY_SERVER
protected:
#else
private:
#endif
	CsFile*							pFile_;
	
private:
	std::string						Name_;
	BcAtomicU32						RefCount_;
	BcAtomicMutex					Lock_;
	CsResourceStage					Stage_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

#endif

