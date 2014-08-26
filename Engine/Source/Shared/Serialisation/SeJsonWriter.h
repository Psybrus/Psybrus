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
	SeJsonWriter( const char* FileName, BcU32 IncludeFieldFlags = 0xffffffff, BcU32 ExcludeFieldFlags = bcRFF_TRANSIENT );
    virtual ~SeJsonWriter();

    virtual BcU32 getVersion() const;
    virtual BcU32 getFileVersion() const;

protected:
    virtual void* internalSerialise( void* pData, const ReType* pType );
    virtual std::string internalSerialiseString( void* pData, const ReType* pType );

protected:
    virtual Json::Value serialiseClass( void* pData, const ReClass* pClass, bool StoreID );
    virtual Json::Value serialiseField( void* pData, const ReField* pField );
    virtual Json::Value serialisePointer( void* pData, const ReClass* pClass );
    virtual Json::Value serialiseArray( void* pData, const ReField* pField );
    virtual Json::Value serialiseDict( void* pData, const ReField* pField );

private:
    Json::Value RootValue_;
    Json::Value ObjectsValue_;
	std::map< BcU32, Json::Value >	ObjectValueMap_;

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

	BcBool shouldSerialiseField( BcU32 Flags );

private:
	BcU32 IncludeFieldFlags_;
	BcU32 ExcludeFieldFlags_;
	std::list< SerialiseClass >		SerialiseClasses_;	///!< Classes to serialise.
    std::string						OutputFile_;
};

#endif
