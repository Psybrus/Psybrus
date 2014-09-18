#ifndef __REFLECTION_UTILITY_H__
#define __REFLECTION_UTILITY_H__

#include "Base/BcTypes.h"
#include "Base/BcBinaryData.h"
#include "Base/BcHash.h"
#include "Base/BcName.h"

#include "Reflection/ReMacros.h"
#include "Reflection/ReObjectRef.h"

#include <vector>
#include <map>
#include <list>
#include <atomic>

//////////////////////////////////////////////////////////////////////////
// FieldFlags
enum ReFieldFlags
{
	// Pointer type info.
	bcRFF_POINTER =					0x00000001,		// Pointer type.
	bcRFF_REFERENCE =				0x00000002,		// Reference type.
	bcRFF_OBJECT_REFERENCE =		0x00000004,		// Smart pointer type (ref counted).

	// Basic type info.
	bcRFF_CONST =					0x00000010,		// Const.
	bcRFF_ATOMIC =					0x00000020,		// Atomic type.
	bcRFF_POD =						0x00000040,		// Plain old data.

	// Properties.
	bcRFF_HIDDEN =					0x00000100,		// Hidden from existance, but included in size & offset calculation.
	bcRFF_TRANSIENT =				0x00000200,		// Don't bother serialising generally.
	bcRFF_SHALLOW_COPY =			0x00000400,		// Only perform a shallow copy on this field when using as a basis.
	bcRFF_IMPORTER =				0x00000800,		// Only parsed in when doing importer serialisation.
	bcRFF_REPLICATED =				0x00001000,		// Field is replicated across the network.
	bcRFF_CHUNK_DATA =				0x00002000,		// Data comes from a chunk.
	bcRFF_BASIS =					0x00004000,		// Field is of a basis object.
	bcRFF_DEBUG_EDIT =				0x00008000,		// Allows for the flag to be read and written to by the DsCore post commands

	// Simple deref when traversing.
	bcRFF_SIMPLE_DEREF = bcRFF_POINTER | bcRFF_REFERENCE | bcRFF_OBJECT_REFERENCE,

	// Any pointer type.
	bcRFF_ANY_POINTER_TYPE = bcRFF_POINTER | bcRFF_REFERENCE | bcRFF_OBJECT_REFERENCE,

	// None + all flags. Short hand.
	bcRFF_NONE =					0x00000000,
	bcRFF_ALL =						0xffffffff,
};

//////////////////////////////////////////////////////////////////////////
// TypeTraits
template< typename _Ty >
struct ReTypeTraits
{
	typedef _Ty Type;
	static const BcU32 Flags = 0;
	static const bool IsEnum = std::is_enum< Type >::value;
	static const char* Name()
	{
		return typeid( Type ).name();
	}
};
		
template<>
struct ReTypeTraits< BcU8 >
{
	typedef BcU8 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcU8";
	}
};

template<>
struct ReTypeTraits< BcS8 >
{
	typedef BcS8 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcS8";
	}
};

template<>
struct ReTypeTraits< BcU16 >
{
	typedef BcU16 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcU16";
	}
};

template<>
struct ReTypeTraits< BcS16 >
{
	typedef BcS16 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcS16";
	}
};

template<>
struct ReTypeTraits< BcU32 >
{
	typedef BcU32 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcU32";
	}
};

template<>
struct ReTypeTraits< BcS32 >
{
	typedef BcS32 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcS32";
	}
};

template<>
struct ReTypeTraits< BcU64 >
{
	typedef BcU64 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcU64";
	}
};

template<>
struct ReTypeTraits< BcS64 >
{
	typedef BcS64 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcS64";
	}
};

template<>
struct ReTypeTraits< BcF32 >
{
	typedef BcF32 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcF32";
	}
};

template<>
struct ReTypeTraits< BcF64 >
{
	typedef BcF64 Type;
	static const BcU32 Flags = bcRFF_POD;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "BcF64";
	}
};

template<>
struct ReTypeTraits< std::string >
{
	typedef std::string Type;
	static const BcU32 Flags = 0;
	static const bool IsEnum = false;
	static const char* Name()
	{
		return "string";
	}
};

template< typename _Ty >
struct ReTypeTraits< _Ty* >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_POINTER;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};

template< typename _Ty >
struct ReTypeTraits< _Ty& >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_REFERENCE;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};

template< typename _Ty >
struct ReTypeTraits< ReObjectRef< _Ty, false > >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_OBJECT_REFERENCE;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};
		
template< typename _Ty >
struct ReTypeTraits< ReObjectRef< _Ty, true > >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_OBJECT_REFERENCE;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};

template< typename _Ty >
struct ReTypeTraits< const _Ty* >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_POINTER | bcRFF_CONST;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};

template< typename _Ty >
struct ReTypeTraits< const _Ty& >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_REFERENCE | bcRFF_CONST;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};
		
template< typename _Ty >
struct ReTypeTraits< std::atomic< _Ty > >
{
	typedef _Ty Type;
	static const BcU32 Flags = bcRFF_ATOMIC;
    static const bool IsEnum = ReTypeTraits< Type >::IsEnum;
	static const char* Name()
	{
        return ReTypeTraits< Type >::Name();
	}
};

//////////////////////////////////////////////////////////////////////////
// BaseTypeConversion
template < typename _Ty >
struct ReBaseTypeConversion
{
	typedef _Ty BaseType;
	typedef _Ty CastType;
	static const int precision = std::numeric_limits< BaseType >::digits10;
};

template <>
struct ReBaseTypeConversion< BcU8 >
{
	typedef BcU8 BaseType;
	typedef BcU32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcU16 >
{
	typedef BcU16 BaseType;
	typedef BcU32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcU32 >
{
	typedef BcU32 BaseType;
	typedef BcU32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcU64 >
{
	typedef BcU64 BaseType;
	typedef BcU64 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcS8 >
{
	typedef BcS8 BaseType;
	typedef BcS32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcS16 >
{
	typedef BcS16 BaseType;
	typedef BcS32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcS32 >
{
	typedef BcS32 BaseType;
	typedef BcS32 CastType;
	static const int precision = 0;
};

template <>
struct ReBaseTypeConversion< BcS64 >
{
	typedef BcS64 BaseType;
	typedef BcS64 CastType;
	static const int precision = 0;
};

//////////////////////////////////////////////////////////////////////////
// NoInit
enum ReNoInit
{
	NOINIT
};

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class ReITypeSerialiser;
class IContainerAccessor;
class ReObject;
class RePrimitive;
class ReType;
class ReEnumConstant;
class ReEnum;
class ReField;
class ReClass;
class ReClassSerialiser;

#endif
