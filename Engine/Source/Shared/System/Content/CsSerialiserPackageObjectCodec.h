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
	CsSerialiserPackageObjectCodec( 
		class CsPackage* Package,
		BcU32 IncludeFieldFlags,
		BcU32 ExcludeFieldFlags );

	virtual BcBool shouldSerialiseContents( 
		void* InData, 
		const ReType* InType ) override;

	virtual std::string serialiseAsStringRef( 
		void* InData, 
		const ReType* InType ) override;
	
	virtual BcBool isMatchingField( 
		const class ReField* Field, 
		const std::string& Name ) override;
	
	virtual BcBool shouldSerialiseField( 
		void* InData, 
		const class ReField* Field ) override;

private:
	class CsPackage* Package_;
	BcU32 IncludeFieldFlags_;
	BcU32 ExcludeFieldFlags_;
};


#endif // __CSSERIALISERPACKAGEOBJECTCODEC_H__
