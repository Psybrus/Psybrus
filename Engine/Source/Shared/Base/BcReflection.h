/**************************************************************************
*
* File:		BcReflection.h
* Author:	Neil Richardson
* Ver/Date:	24/12/12
* Description:
*			Reflection. Typing information + attributes.
*			Based mostly on Don Williamson's AltDevBlogADay article:
*			http://www.altdevblogaday.com/2011/09/25/reflection-in-c-part-1-introduction/
*
*
**************************************************************************/

#ifndef __BCREFLECTION_H__
#define __BCREFLECTION_H__

#include "Base/BcTypes.h"
#include "Base/BcHash.h"
#include "Base/BcName.h"
#include "Base/BcGlobal.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class BcReflectionPrimitive;
class BcReflectionType;
class BcReflectionEnumConstant;
class BcReflectionEnum;
class BcReflectionField;
class BcReflectionClass;
class BcReflection;

//////////////////////////////////////////////////////////////////////////
// RTTI Defines.
#define __BCREFLECTION_DECLARE_BASE( _Type )									\
	public:																		\
	static const BcName& StaticGetType();										\
	static BcHash StaticGetTypeHash();											\
	static const BcReflectionClass* StaticGetClass();							\
	static void StaticRegisterReflection();										\
	virtual const BcName& getTypeName() const;									\
	virtual BcHash getTypeHash() const;											\
	virtual const BcReflectionClass* getClass() const;							\
	virtual BcBool isType( const BcName& Type ) const;							\
	virtual BcBool isTypeOf( const BcName& Type ) const;						\
	template < class _Ty >														\
	BcForceInline BcBool isTypeOf() const										\
	{																			\
		return this ? isTypeOf( _Ty::StaticGetType() ) : BcFalse;				\
	}

#define __BCREFLECTION_DEFINE_BASE( _Type )										\
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
	const BcReflectionClass* _Type::StaticGetClass()							\
	{																			\
		static const BcReflectionType* pType = BcReflection::pImpl()->getType( StaticGetType() ); \
		static const BcReflectionClass* pClass = pType->isTypeOf< BcReflectionClass >() ? static_cast< const BcReflectionClass* >( pType ) : NULL; \
		return pClass;															\
	}																			\
	const BcName& _Type::getTypeName() const 									\
	{																			\
		return _Type::StaticGetType();											\
	}																			\
																				\
	const BcReflectionClass* _Type::getClass() const							\
	{																			\
		return _Type::StaticGetClass();											\
	}

#define BCREFLECTION_DECLARE_BASE( _Type )										\
	__BCREFLECTION_DECLARE_BASE( _Type )										\
	BcForceInline _Type* getSuper()												\
	{																			\
		return NULL;															\
	}																			\

#define BCREFLECTION_DEFINE_BASE( _Type )										\
	__BCREFLECTION_DEFINE_BASE( _Type )											\
	BcHash _Type::getTypeHash() const											\
	{																			\
		return _Type::StaticGetTypeHash();										\
	}																			\
																				\
	BcBool _Type::isType( const BcName& Type ) const							\
	{																			\
		return _Type::StaticGetType() == Type;									\
	}																			\
																				\
	BcBool _Type::isTypeOf( const BcName& Type ) const 							\
	{																			\
		return _Type::StaticGetType() == Type;									\
	}																			\

#define BCREFLECTION_DECLARE_DERIVED( _Base, _Type )							\
	public:																		\
		typedef _Base Super;													\
		__BCREFLECTION_DECLARE_BASE( _Type )									\
		BcForceInline Super* getSuper()											\
		{																		\
			return this;														\
		}																		\

#define BCREFLECTION_DEFINE_DERIVED( _Type )									\
	__BCREFLECTION_DEFINE_BASE( _Type )											\
	BcHash _Type::getTypeHash() const 											\
	{																			\
		return _Type::StaticGetTypeHash();										\
	}																			\
																				\
	BcBool _Type::isType( const BcName& Type ) const							\
	{																			\
		return  _Type::StaticGetType() == Type;									\
	}																			\
																				\
	BcBool _Type::isTypeOf( const BcName& Type ) const 							\
	{																			\
		return _Type::StaticGetType() == Type || Super::isTypeOf( Type );		\
	}

//////////////////////////////////////////////////////////////////////////
// BCREFLECTION_BEGIN
#define BCREFLECTION_EMPTY_REGISTER( _Type )									\
	void _Type::StaticRegisterReflection()										\
	{																			\
		typedef _Type ThisType;													\
		static BcReflectionClass Class( ThisType::StaticGetType(),				\
		                                sizeof( ThisType ),						\
		                                Super::StaticGetType(),					\
		                                NULL,									\
		                                0 );									\
		BcReflection::pImpl()->addType( &Class );								\
	}

#define BCREFLECTION_BASE_BEGIN( _Type )										\
	void _Type::StaticRegisterReflection()										\
	{																			\
		typedef _Type ThisType;													\
		static BcReflectionField Fields[] =										\
		{

#define BCREFLECTION_BASE_END()													\
		};																		\
		static BcReflectionClass Class( ThisType::StaticGetType(),				\
		                                sizeof( ThisType ),						\
		                                BcName::NONE,							\
		                                Fields,									\
		                                BcArraySize( Fields ) );				\
		BcReflection::pImpl()->addType( &Class );								\
	}

#define BCREFLECTION_DERIVED_BEGIN( _Base, _Type )								\
	void _Type::StaticRegisterReflection()										\
	{																			\
		typedef _Base BaseType;													\
		typedef _Type ThisType;													\
		static BcReflectionField Fields[] =										\
		{

#define BCREFLECTION_DERIVED_END()												\
		};																		\
		static BcReflectionClass Class( ThisType::StaticGetType(),				\
		                                sizeof( ThisType ),						\
		                                BaseType::StaticGetType(),				\
		                                Fields,									\
		                                BcArraySize( Fields ) );				\
		BcReflection::pImpl()->addType( &Class ); \
	}

#define BCREFLECTION_MEMBER( _Type, _Member, _Flags )							\
	BcReflectionField( #_Member, #_Type, BcOffsetOf( ThisType, _Member ), _Flags )

//////////////////////////////////////////////////////////////////////////
// BcReflectionPrimitive
class BcReflectionPrimitive
{
	BCREFLECTION_DECLARE_BASE( BcReflectionPrimitive );

public:
	BcReflectionPrimitive( const BcName& Name );
	virtual ~BcReflectionPrimitive(){};

	/**
	 * Get primitive's name.
	 */
	const BcName&					getName() const;

	/**
	 * Get primitive's name hash.
	 */
	const BcHash&					getNameHash() const;

	/**
	 * Log the primitive.
	 */
	virtual void					log() const{};

protected:
	BcName							Name_;
	BcHash							NameHash_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionFieldFlags
enum BcReflectionFieldFlags
{
	bcRFF_DEFAULT =					0x00000000,
	bcRFF_POINTER =					0x00000001,		// Pointer type.
	bcRFF_REFERENCE =				0x00000004,		// Reference type or wrapped pointer.
	bcRFF_TRANSIENT =				0x00000002,		// Transient, shouldn't be saved/loaded.
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionType
class BcReflectionType: 
	public BcReflectionPrimitive
{
	BCREFLECTION_DECLARE_DERIVED( BcReflectionPrimitive, BcReflectionType );

	BcU32							getSize() const;

public:
	BcReflectionType( const BcName& Name, BcU32 Size );
	virtual ~BcReflectionType(){};

protected:
	BcU32							Size_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionEnumConstant
class BcReflectionEnumConstant: 
	public BcReflectionPrimitive
{
	BCREFLECTION_DECLARE_DERIVED( BcReflectionPrimitive, BcReflectionEnumConstant );

public:
	BcReflectionEnumConstant( const BcName& Name, BcU32 Value );
	virtual ~BcReflectionEnumConstant(){};

protected:
	BcU32							Value_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionEnum
class BcReflectionEnum: 
	public BcReflectionType
{
	BCREFLECTION_DECLARE_DERIVED( BcReflectionType, BcReflectionEnum );

public:
	BcReflectionEnum( const BcName& Name, BcU32 Size );
	virtual ~BcReflectionEnum(){};

protected:
	const BcReflectionEnumConstant*	pEnumConstants_;
	BcU32							NoofEnumConstants_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionField
class BcReflectionField: 
	public BcReflectionPrimitive
{
	BCREFLECTION_DECLARE_DERIVED( BcReflectionPrimitive, BcReflectionField );

public:
	BcReflectionField( const BcName& Name, const BcName& Type, BcU32 Offset, BcU32 Flags );
	virtual ~BcReflectionField(){};

	const BcReflectionType*			getType() const;
	BcU32							getOffset() const;
	BcU32							getFlags() const;

protected:
	BcName							Type_;
	BcU32							Offset_;
	BcU32							Flags_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflectionClass
class BcReflectionClass: 
	public BcReflectionType
{
	BCREFLECTION_DECLARE_DERIVED( BcReflectionType, BcReflectionClass );

public:
	BcReflectionClass( const BcName& Name, BcU32 Size, const BcName Super, const BcReflectionField* pFields, BcU32 NoofFields );
	virtual ~BcReflectionClass(){};

	/**
	 * Get super.
	 */
	const BcReflectionClass*		getSuper() const;

	/**
	 * Get field.
	 */
	const BcReflectionField*		getField( BcU32 Idx ) const;

	/**
	 * Get noof fields.
	 */
	BcU32							getNoofFields() const;

	/**
	 * Log out fields. Temporary for debugging.
	 */
	virtual void					log() const;

protected:
	BcName							Super_;
	const BcReflectionField*		pFields_;
	BcU32							NoofFields_;
};

//////////////////////////////////////////////////////////////////////////
// BcReflection
class BcReflection:
	public BcGlobal< BcReflection >
{
public:
	BcReflection();
	virtual ~BcReflection();

	/**
	 * Add a new type.
	 */
	void							addType( const BcReflectionType* pType );

	/**
	 * Get a type.
	 */
	const BcReflectionType*			getType( const BcName& Type );

protected:
	typedef std::map< BcName, const BcReflectionType* > TTypeMap;
		
	TTypeMap						Types_;
};

#endif
