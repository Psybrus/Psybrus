#ifndef __REFLECTION_MACROS_H__
#define __REFLECTION_MACROS_H__

#include <string>

//////////////////////////////////////////////////////////////////////////
// Reflection Defines

/**
 * Internal
 */
#define __REFLECTION_DECLARE_BASIC( _Type )										\
	public:																		\
	static BcName StaticGetTypeName();								\
	static BcU32 StaticGetTypeNameHash();										\
	static const ReClass* StaticGetClass();										\
	static void StaticRegisterClass();											\
	friend class ReClassSerialiser_##_Type;										\

/**
 * Internal
 */
#define __REFLECTION_DEFINE_BASIC( _Type )										\
	BcName _Type::StaticGetTypeName()								\
	{																			\
		static std::string Name( ReTypeTraits< _Type >::Name() );				\
		return Name;															\
	}																			\
																				\
	BcU32 _Type::StaticGetTypeNameHash()										\
	{																			\
		static BcU32 TypeHash = BcHash( (*StaticGetTypeName()).c_str() );		\
		return TypeHash;														\
	}																			\
																				\
	const ReClass* _Type::StaticGetClass()										\
	{																			\
		static const ReClass* pClass = ReManager::GetClass( StaticGetTypeName() );\
		return pClass;															\
	}																			\

/**
 * Internal
 */
#define __REFLECTION_DECLARE_BASE( _Type )										\
	__REFLECTION_DECLARE_BASIC( _Type )											\
	virtual BcName getTypeName() const;											\
	virtual BcU32 getTypeHash() const;											\
	virtual const ReClass* getClass() const;									\
	virtual BcBool isType( BcName Type ) const;									\
	virtual BcBool isTypeOf( const ReClass* pClass ) const;						\
	template < class _Ty >														\
	inline BcBool isTypeOf() const												\
	{																			\
		return this ? isTypeOf( _Ty::StaticGetClass() ) : false;				\
	}																			\

/**
 * Internal
 */
#define __REFLECTION_DEFINE_BASE( _Type )										\
	__REFLECTION_DEFINE_BASIC( _Type )											\
	BcName _Type::getTypeName() const								\
	{																			\
		return _Type::StaticGetTypeName();										\
	}																			\
																				\
	const ReClass* _Type::getClass() const										\
	{																			\
		return _Type::StaticGetClass();											\
	}																			\

/**
 * @brief Declare basic type.
 * 
 * Used for POD types that don't want a v-table.
 * Should be put in the header inside the class definition.
 */
#define REFLECTION_DECLARE_BASIC( _Type )										\
	__REFLECTION_DECLARE_BASIC( _Type )											\
	_Type( ReNoInit ){};

/**
 * @brief Declare basic type, no autoreg.
 * 
 * Used for POD types that don't want a v-table.
 * Should be put in the header inside the class definition.
 */
#define REFLECTION_DECLARE_BASIC_NOAUTOREG( _Type )								\
	REFLECTION_DECLARE_BASIC( _Type )											\


/**
 * @brief Define basic type.
 * 
 * Used for POD types that don't want a v-table.
 * Should be put in the cpp.
 */
#define REFLECTION_DEFINE_BASIC( _Type )										\
	__REFLECTION_DEFINE_BASIC( _Type )											\


/**
 * @brief Declare base type.
 * 
 * Used for base types that want RTTI support.
 */
#define REFLECTION_DECLARE_BASE( _Type )										\
	__REFLECTION_DECLARE_BASE( _Type )											\
	_Type( ReNoInit ){};

/**
 * @brief Declare base type, no autoreg.
 * 
 * Used for base types that want RTTI support.
 */
#define REFLECTION_DECLARE_BASE_NOAUTOREG( _Type )								\
	REFLECTION_DECLARE_BASE( _Type )											\

/**
 * @brief Declare base type with manual NoInit.
 * 
 * Used for base types that want RTTI support.
 * Should be put in the header in the class definition.
 * Must define you own constructor with constructor:
 *   TYPE( NoInit )
 */
#define REFLECTION_DECLARE_BASE_MANUAL_NOINIT( _Type )							\
	__REFLECTION_DECLARE_BASE( _Type )											\

/**
 * @brief Define base type.
 * 
 * Used for base types that want RTTI support.
 * Should be put in the cpp.
 */
#define REFLECTION_DEFINE_BASE( _Type )											\
	__REFLECTION_DEFINE_BASE( _Type )											\
	BcU32 _Type::getTypeHash() const											\
	{																			\
		return _Type::StaticGetTypeNameHash();									\
	}																			\
																				\
	BcBool _Type::isType( BcName Type ) const									\
	{																			\
		return _Type::StaticGetTypeName() == Type;								\
	}																			\
																				\
	BcBool _Type::isTypeOf( const ReClass* pClass ) const 						\
	{																			\
		return _Type::StaticGetClass() == pClass;								\
	}																			\

/**
 * @brief Declare derived type.
 * 
 * Used for derived types that want RTTI support.
 * Should be put in the header in the class definition.
 */
#define REFLECTION_DECLARE_DERIVED( _Type, _Base )								\
	public:																		\
		typedef _Base Super;													\
		__REFLECTION_DECLARE_BASE( _Type )										\
	_Type( ReNoInit ): _Base( NOINIT ) {};

/**
 * @brief Declare derived type, no autored
 * 
 * Used for derived types that want RTTI support.
 * Should be put in the header in the class definition.
 */
#define REFLECTION_DECLARE_DERIVED_NOAUTOREG( _Type, _Base )					\
		REFLECTION_DECLARE_DERIVED( _Type, _Base )								\

/**
 * @brief Declare derived type.
 * 
 * Used for derived types that want RTTI support.
 * Should be put in the header in the class definition.
  * Must define you own constructor with constructor:
 *   TYPE( NoInit )
 */
#define REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( _Type, _Base )				\
	public:																		\
		typedef _Base Super;													\
		__REFLECTION_DECLARE_BASE( _Type )										\

/**
 * @brief Define derived type.
 * 
 * Used for derived types that want RTTI support.
 * Should be put in the cpp.
 */
#define REFLECTION_DEFINE_DERIVED( _Type )										\
	__REFLECTION_DEFINE_BASE( _Type )											\
	BcU32 _Type::getTypeHash() const											\
	{																			\
		return _Type::StaticGetTypeNameHash();									\
	}																			\
																				\
	BcBool _Type::isType( BcName Type ) const									\
	{																			\
		return  _Type::StaticGetTypeName() == Type;								\
	}																			\
																				\
	BcBool _Type::isTypeOf( const ReClass* pClass ) const 						\
	{																			\
		return _Type::StaticGetClass() == pClass || Super::isTypeOf( pClass );	\
	}																			\

#endif
