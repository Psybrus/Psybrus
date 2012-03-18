/**************************************************************************
*
* File:		RmParameters.h
* Author:	Neil Richardson 
* Ver/Date:	9/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RMPARAMETERS_H__
#define __RMPARAMETERS_H__

#include "Base/BcTypes.h"
#include "Base/BcStream.h"
#include "Base/BcDebug.h"

////////////////////////////////////////////////////////////////////////////////
// RmParameters
class RmParameters
{
public:
	enum eType
	{
		TYPE_SEND,
		TYPE_RECV,
	};
	
public:
	RmParameters( eType Type, void* pData = NULL, BcU32 Size = 0 );
	~RmParameters();
	
	/**
	 * Push data.
	 */
	void push( const void* pData, BcU32 Bytes );

	/**
	 * Pop data.
	 */
	void pop( void* pData, BcU32 Bytes );

	/**
	 * String serialise out operator.
	 */
	RmParameters& operator << ( const std::string& Value );

	/**
	 * String serialise in operator.
	 */
	RmParameters& operator >> ( std::string& Value );

	/**
	 * Generic serialise out operator.
	 */
	template< typename _Ty >
	RmParameters& operator << ( const _Ty& Value )
	{
		push( &Value, sizeof( _Ty ) );
		return (*this);
	}
	
	/**
	 * Generic serialise in operator.
	 */
	template< typename _Ty >
	RmParameters& operator >> ( _Ty& Value )
	{
		pop( &Value, sizeof( _Ty ) );
		return (*this);
	}
	
	/**
	 * Get raw data.
	 */
	void* getData();
	
	/**
	 * Get data size.
	 */
	BcU32 getDataSize() const;
	
	/**
	 * Get data from cursor.
	 */
	void* getDataFromCursor();
	
	/**
	 * Get size from cursor.
	 */
	BcU32 getDataSizeFromCursor() const;
	

private:
	void resize( BcU32 NewSize );

private:
	eType Type_;
	BcU32 Cursor_;
	BcU8* pBuffer_;
	BcU32 BufferSize_;
};


#endif
