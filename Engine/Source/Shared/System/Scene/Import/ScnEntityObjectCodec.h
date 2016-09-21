/**************************************************************************
*
* File:		ScnEntityObjectCodec.h
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __SCNENTITYOBJECTCODEC_H__
#define __SCNENTITYOBJECTCODEC_H__

#include "Serialisation/SeSerialisation.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntityObjectCodec
class ScnEntityObjectCodec:
	public SeISerialiserObjectCodec
{
public:
	/**
	 * Construct package object codec.
	 * @param RootEntity Root entity we are serialising.
	 * @param IncludeFieldFlags What fiags a field must have to be serialised.
	 * @param ExcludeFieldFlags What flags a field must not have to be serialised.
	 * @param PropagateFieldFlags What flags propagate from a field to its members.
	 */
	ScnEntityObjectCodec( class ScnEntity* RootEntity );

	virtual BcBool shouldSerialiseContents( 
		void* InData, 
		const ReClass* InType ) override;

	virtual std::string serialiseAsStringRef( 
		void* InData, 
		const ReClass* InType ) override;
	
	virtual BcBool isMatchingField( 
		const class ReField* Field, 
		const std::string& Name ) override;
	
	virtual BcBool shouldSerialiseField( 
		void* InData, 
		BcU32 ParentFlags,
		const class ReField* Field ) override;

	virtual BcBool findObject( 
		void*& OutObject, const ReClass* Type, BcU32 Key ) override;

	virtual BcBool findObject( 
		void*& OutObject, const ReClass* Type, const std::string& Key ) override;
	
private:
	class ScnEntity* RootEntity_;
	BcU32 IncludeFieldFlags_;
	BcU32 ExcludeFieldFlags_;
	BcU32 PropagateFieldFlags_;
};


#endif // __SCNENTITYOBJECTCODEC_H__
