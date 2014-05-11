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
    static const char* SerialiserVersionEntry;
    static const char* RootIDEntry;
    static const char* ObjectsEntry;
    static const char* ClassEntry;
    static const char* IDEntry;
    static const char* MembersEntry;
    static const char* FieldEntry;
    static const char* ValueEntry;

public:
    SeJsonWriter( const char* FileName );
    virtual ~SeJsonWriter();

    virtual BcU32 getVersion() const;
    virtual BcU32 getFileVersion() const;

protected:
    virtual void* internalSerialise( void* pData, const ReType* pType );

protected:
    virtual Json::Value serialiseClass( void* pData, const ReClass* pClass, bool StoreID );
    virtual Json::Value serialiseField( void* pData, const ReField* pField );
    virtual Json::Value serialisePointer( void* pData, const ReClass* pClass );
    virtual Json::Value serialiseArray( void* pData, const ReField* pField );
    virtual Json::Value serialiseDict( void* pData, const ReField* pField );

private:
    Json::Value RootValue_;
    Json::Value ObjectsValue_;

    struct SerialiseClass
    {
        SerialiseClass( void* pData, const ReType* pType ):
            pData_( pData ),
            pType_( pType )
        {
        }

        void*						pData_;
        const ReType*		pType_;

        bool operator == ( const SerialiseClass& Other )
        {
            return pData_ == Other.pData_ && pType_ == Other.pType_;
        }
    };

private:
    std::list< SerialiseClass >		SerialiseClasses_;	///!< Classes to serialise.
    std::vector< Json::Value >		OutputValues_;		///!< Values to write out.
    std::string						OutputFile_;
};

#endif
