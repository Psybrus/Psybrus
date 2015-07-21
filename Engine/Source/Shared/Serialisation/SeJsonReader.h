#ifndef __JsonReader_H__
#define __JsonReader_H__

#include "Serialisation/SeSerialisation.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// SeJsonReader
class SeJsonReader:
	public SeISerialiser
{
public:
	SeJsonReader( 
		SeISerialiserObjectCodec* ObjectCodec );
	BcBool load( std::string FileName );
	virtual ~SeJsonReader();

	virtual BcU32 getVersion() const;
	virtual BcU32 getFileVersion() const;

protected:
	virtual void* internalSerialise( void* pData, const ReClass* pType );
	virtual std::string internalSerialiseString( void* pData, const ReClass* pType );

public:
	virtual void serialiseClass( void* pData, const ReClass* pClass, const Json::Value& InputValue, BcU32 ParentFlags );
	virtual void serialiseClassMembers( void* pData, const ReClass* pClass, const Json::Value& MemberValues, BcU32 ParentFlags );
	virtual void serialiseField( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags );
	virtual void serialisePointer( void*& pData, const ReClass* pClass, BcU32 FieldFlags, const Json::Value& InputValue, BcU32 ParentFlags, BcBool IncrementRefCount );
	virtual void serialiseArray( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags );
	virtual void serialiseDict( void* pData, const ReField* pField, const Json::Value& InputValue, BcU32 ParentFlags );
  
private:
	Json::Value RootValue_;

	struct SerialiseClass
	{
		SerialiseClass( std::string ID, void* pData, const ReClass* pType ):
			ID_( ID ),
			pData_( pData ),
			pType_( pType )
		{
		}

		std::string ID_;
		void* pData_;
		const ReClass* pType_;

		bool operator == ( const SerialiseClass& Other )
		{
			return ID_ == Other.ID_;
		}
	};

	SerialiseClass getSerialiseClass( BcU32 ID, const ReClass* pType );
	SerialiseClass getSerialiseClass( std::string ID, const ReClass* pType );
	SerialiseClass getSerialiseClass( const Json::Value& Value, const ReClass* pType );

private:
	SeISerialiserObjectCodec* ObjectCodec_;
	BcU32 FileVersion_;
	std::list< SerialiseClass > SerialiseClasses_;	///!< Classes to serialise.
	std::vector< Json::Value > InputValues_;		///!< Values to read int.
};

#endif
