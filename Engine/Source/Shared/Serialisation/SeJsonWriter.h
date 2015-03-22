#ifndef __JsonWriter_H__
#define __JsonWriter_H__

#include "Serialisation/SeSerialisation.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// SeJsonWriter
class SeJsonWriter:
    public SeISerialiser
{
public:
	SeJsonWriter( 
		SeISerialiserObjectCodec* ObjectCodec );
    virtual ~SeJsonWriter();
	void save( std::string FileName );
    virtual BcU32 getVersion() const;
    virtual BcU32 getFileVersion() const;

protected:
    virtual void* internalSerialise( void* pData, const ReClass* pType );
    virtual std::string internalSerialiseString( void* pData, const ReClass* pType );

protected:
    virtual Json::Value serialiseClass( void* pData, const ReClass* pClass, BcU32 ParentFlags, bool StoreID );
    virtual Json::Value serialiseField( void* pData, const ReField* pField, BcU32 ParentFlags );
    virtual Json::Value serialisePointer( void* pData, const ReClass* pClass, BcU32 ParentFlags );
    virtual Json::Value serialiseArray( void* pData, const ReField* pField, BcU32 ParentFlags );
    virtual Json::Value serialiseDict( void* pData, const ReField* pField, BcU32 ParentFlags );

private:
    Json::Value RootValue_;
    Json::Value ObjectsValue_;
	std::map< std::string, Json::Value > ObjectValueMap_;

    struct SerialiseClass
    {
        SerialiseClass( void* pData, const ReClass* pType ):
            pData_( pData ),
            pType_( pType )
        {
        }

        void*						pData_;
        const ReClass*		pType_;

        bool operator == ( const SerialiseClass& Other )
        {
            return pData_ == Other.pData_ && pType_ == Other.pType_;
        }
    };

private:
	SeISerialiserObjectCodec* ObjectCodec_;
	std::list< SerialiseClass > SerialiseClasses_;	///!< Classes to serialise.
	std::string Output_;
};

#endif
