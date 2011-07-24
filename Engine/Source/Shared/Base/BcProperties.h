/**************************************************************************
*
* File:		BcProperties.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*  
*
*
* 
**************************************************************************/

#ifndef __BCPROPERTIES_H__
#define __BCPROPERTIES_H__

#include "BcTypes.h"
#include "BcVectors.h"
#include "BcMat3d.h"
#include "BcMat4d.h"

#include <string>

//////////////////////////////////////////////////////////////////////////
// BcPropertyType
enum BcPropertyType
{
	bcPT_NULL = 0,
	bcPT_INT,
	bcPT_REAL,
	bcPT_VEC2D,
	bcPT_VEC3D,
	bcPT_VEC4D,
	bcPT_MAT3D,
	bcPT_MAT4D,
	bcPT_STRING
};

//////////////////////////////////////////////////////////////////////////
// BcProperty
struct BcProperty
{
	const BcChar*		pName_;
	BcU32				Offset_;
	BcPropertyType		Type_;
};

//////////////////////////////////////////////////////////////////////////
// BcPropertyTrait
template< typename _Ty >
struct BcPropertyTrait;

template<>
struct BcPropertyTrait< BcU32 >
{
	static const BcPropertyType TYPE = bcPT_INT;
};

template<>
struct BcPropertyTrait< BcS32 >
{
	static const BcPropertyType TYPE = bcPT_INT;
};

template<>
struct BcPropertyTrait< BcReal >
{
	static const BcPropertyType TYPE = bcPT_REAL;
};

template<>
struct BcPropertyTrait< BcVec2d >
{
	static const BcPropertyType TYPE = bcPT_VEC2D;
};

template<>
struct BcPropertyTrait< BcVec3d >
{
	static const BcPropertyType TYPE = bcPT_VEC3D;
};

template<>
struct BcPropertyTrait< BcVec4d >
{
	static const BcPropertyType TYPE = bcPT_VEC4D;
};

template<>
struct BcPropertyTrait< BcMat3d >
{
	static const BcPropertyType TYPE = bcPT_MAT3D;
};

template<>
struct BcPropertyTrait< BcMat4d >
{
	static const BcPropertyType TYPE = bcPT_MAT4D;
};

template<>
struct BcPropertyTrait< std::string >
{
	static const BcPropertyType TYPE = bcPT_STRING;
};

//////////////////////////////////////////////////////////////////////////
// BcPropertyTypeGetter
template < typename _Ty >
BcForceInline BcPropertyType BcPropertyTypeGetter( _Ty* Value )
{
	(void)Value;
	return BcPropertyTrait< _Ty >::TYPE;
}

//////////////////////////////////////////////////////////////////////////
// Macros
#define BCPROPERTIES_DECLARE()													\
	private:																	\
		static BcProperty Properties_[];										\
																				\
		BcProperty* findProperty( const BcChar* pName )							\
		{																		\
			BcProperty* pProperty = &TestPropertyObject::Properties_[0];		\
			while( pProperty->Type_ != bcPT_NULL )								\
			{																	\
				if( BcStrCompare( pProperty->pName_, pName ) )					\
					return pProperty;											\
				++pProperty;													\
			}																	\
			return NULL;														\
		}																		\
																				\
		template< typename _Ty >												\
		void setProperty( const BcChar* pName, const _Ty& Value )				\
		{																		\
																				\
		}
	
#define BCPROPERTIES_BEGIN( _ClassName )										\
	BcProperty _ClassName::Properties_[] =										\
	{

#define BCPROPERTY_ENTRY( _ClassName, _Name, _Member )							\
		{                                                                       \
			_Name,                                                              \
			BcOffsetOf( _ClassName, _Member ),                                  \
			BcPropertyTypeGetter( &(((_ClassName*)0)->_Member) )                \
		}

#define BCPROPERTIES_END( _ClassName )											\
		{ NULL, 0, bcPT_NULL }													\
	}


#endif

