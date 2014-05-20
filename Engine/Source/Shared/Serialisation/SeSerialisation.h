#ifndef __SERIALISATION_H__
#define __SERIALISATION_H__

#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// SERIALISER_VERSION
static const int SERIALISER_VERSION = 1;

//////////////////////////////////////////////////////////////////////////
// SeISerialiserPointerCodec
class SeISerialiserPointerCodec
{
public:
    SeISerialiserPointerCodec(){}
    virtual ~SeISerialiserPointerCodec(){};

    /**
     * @brief Encodes a pointer as a string.
     * @param InData Pointer to data.
     * @param InClass Class that data represents.
     * @return String ID.
     */
    virtual std::string encodePointer( void* InData, const ReClass* InClass ) = 0;

    /**
     * @brief Encodes a pointer as a string.
     * @param InID ID to decode.
     * @param OutData Pointer to data.
     * @param OutClass Class that data represents.
     * @return Success.
     */
    virtual bool decodePointer( const std::string& InID, void*& OutData, const ReClass*& OutClass ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// SeISerialiser
class SeISerialiser
{
public:
    SeISerialiser(){}
    virtual ~SeISerialiser(){};

    /**
     * @brief Get version we should serialise at.
     */
    virtual BcU32 getVersion() const = 0;

    /**
     * @brief Get version the file is at.
     */
    virtual BcU32 getFileVersion() const = 0;

	/**
	 * @brief Serialises class to string
	 * @param pData Pointer to class data
	 * @param pType Type to serialise as.
	 */
	template< typename _Ty >
    std::string serialiseToString( _Ty* pData, const ReType* pType )
    {
        return ( internalSerialiseString( reinterpret_cast< void* >( pData ), pType ) );
    }

    /**
     * @brief Serialise class.
     * @param pData Pointer to class data
     * @param pType Type to serialise as.
     */
    template< typename _Ty >
    _Ty* serialise( _Ty* pData, const ReType* pType )
    {
        return reinterpret_cast< _Ty* >( internalSerialise( reinterpret_cast< void* >( pData ), pType ) );
    }

    /**
     * @brief Serialise data. Wrapper to 'serialise'
     * @param Input Input instance.
     */
    template< typename _Ty >
    SeISerialiser& operator << ( _Ty& Input )
    {
        internalSerialise( &Input, _Ty::StaticGetClass() );
        return (*this);
    }

    template< typename _Ty >
    SeISerialiser& operator << ( _Ty*& Input )
    {
        Input = internalSerialise( reinterpret_cast< void* >( Input ), _Ty::StaticGetClass() );
        return (*this);
    }

protected:
    /**
     * @brief Serialise class.
     * @param pData Pointer to class data
     * @param pType Type to serialise as.
     * @return New data pointer (may be the same)
     */
    virtual void* internalSerialise( void* pData, const ReType* pType ) = 0;
    /**
     * @brief Serialise class to std::string.
     * @param pData Pointer to class data
     * @param pType Type to serialise as.
     * @return std::string Data as string
     */
    virtual std::string internalSerialiseString( void* pData, const ReType* pType ) = 0;
};

#endif
