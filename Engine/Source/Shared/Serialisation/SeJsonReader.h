#ifndef __JsonReader_H__
#define __JsonReader_H__

#include "Serialisation/SeSerialisation.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// SeJsonReader
class SeJsonReader:
    public SeISerialiser,
    public SeISerialiserPointerCodec
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
    SeJsonReader( const char* FileName );
    virtual ~SeJsonReader();

    virtual BcU32 getVersion() const;
    virtual BcU32 getFileVersion() const;

protected:
    virtual void* internalSerialise( void* pData, const ReType* pType );

protected:
    virtual void serialiseClass( void* pData, const ReClass* pClass, Json::Value& InputValue );
    virtual void serialiseField( void* pData, const ReField* pField, Json::Value& InputValue );
    virtual void serialisePointer( void*& pData, const ReClass* pClass, BcU32 FieldFlags, Json::Value& InputValue, BcBool IncrementRefCount );
    virtual void serialiseArray( void* pData, const ReField* pField, Json::Value& InputValue );
    virtual void serialiseDict( void* pData, const ReField* pField, Json::Value& InputValue );

private:
    Json::Value RootValue_;

    struct SerialiseClass
    {
        SerialiseClass( size_t ID, void* pData, const ReType* pType ):
            ID_( ID ),
            pData_( pData ),
            pType_( pType )
        {
        }

        size_t						ID_;
        void*						pData_;
        const ReType*		pType_;

        bool operator == ( const SerialiseClass& Other )
        {
            return ID_ == Other.ID_;
        }
    };

    SerialiseClass					getSerialiseClass( size_t ID, const ReType* pType );

private:
    BcU32							FileVersion_;
    std::list< SerialiseClass >		SerialiseClasses_;	///!< Classes to serialise.
    std::vector< Json::Value >		InputValues_;		///!< Values to read int.
    std::string						InputFile_;
};

#endif
