/**************************************************************************
*
* File:		CsSerialiserPackageObjectCodec.h
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSSERIALISERPACKAGEOBJECTCODEC_H__
#define __CSSERIALISERPACKAGEOBJECTCODEC_H__

#include "Serialisation/SeSerialisation.h"

//////////////////////////////////////////////////////////////////////////
// CsSerialiserPackageObjectCodec
class CsSerialiserPackageObjectCodec:
	public SeISerialiserObjectCodec
{
public:
	CsSerialiserPackageObjectCodec( class CsPackage* Package );
	virtual BcBool shouldSerialise( void* InData, const ReType* InType );
	virtual std::string serialiseAsStringRef( void* InData, const ReType* InType );
	virtual BcBool isMatchingField( const class ReField* Field, const std::string& Name );


private:
	class CsPackage* Package_;
};


#endif // __CSSERIALISERPACKAGEOBJECTCODEC_H__
