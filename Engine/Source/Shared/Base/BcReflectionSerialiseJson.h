/**************************************************************************
*
* File:		BcReflectionSerialiseJson.h
* Author:	Neil Richardson
* Ver/Date:	28/12/12
* Description:
*			Reflection serialisation.
*
*
*
*
**************************************************************************/

#ifndef __BCREFLECTIONSERIALISEJSON_H__
#define __BCREFLECTIONSERIALISEJSON_H__

#include "BcReflectionSerialise.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// BcReflectionSerialiseJson
class BcReflectionSerialiseJson:
	public BcReflectionSerialise
{
public:
	template< typename _Ty >
	BcForceInline BcReflectionSerialiseJson( _Ty* pData )
	{
		RootValue_ = Json::Value( Json::objectValue );
		pCurrValue_ = &RootValue_;

		serialise< _Ty >( pData );
	}

	virtual ~BcReflectionSerialiseJson();

	const Json::Value&				getRootValue() const;

protected:
	virtual void					serialiseBeginClass( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseEndClass( void* pData, const BcReflectionClass* pClass );
	virtual void					serialiseField( void* pData, const BcReflectionClass* pParentClass, const BcReflectionField* pField );

protected:
	Json::Value						getValue( void* pData, const BcReflectionField* pField );

protected:
	Json::Value						RootValue_;
	std::list< Json::Value* >		ValueStack_;
	std::list< BcName >				ClassStack_;

	Json::Value*					pCurrValue_;
	std::string						CurrMember_;
	BcName							CurrClass_;
};

#endif
