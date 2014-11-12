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
	/**
	 * Construct package object codec.
	 * @param Package Package we are serialising.
	 * @param IncludeFieldFlags What fiags a field must have to be serialised.
	 * @param ExcludeFieldFlags What flags a field must not have to be serialised.
	 * @param PropagateFieldFlags What flags propagate from a field to its members.
	 */
	CsSerialiserPackageObjectCodec( 
		class CsPackage* Package,
		BcU32 IncludeFieldFlags,
		BcU32 ExcludeFieldFlags,
		BcU32 PropagateFieldFlags );

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
		BcU32 ParentFlags,
		const class ReField* Field ) override;

private:
	class CsPackage* Package_;
	BcU32 IncludeFieldFlags_;
	BcU32 ExcludeFieldFlags_;
	BcU32 PropagateFieldFlags_;
};


#endif // __CSSERIALISERPACKAGEOBJECTCODEC_H__
