/**************************************************************************
*
* File:		BcCompression.cpp
* Author: 	Neil Richardson 
* Ver/Date:	9/04/2012
* Description:
*		
*		
*		
* 
**************************************************************************/

#include "BcCompression.h"

#include <zlib.h>

//////////////////////////////////////////////////////////////////////////
// BcCompressData
BcBool BcCompressData( const BcU8* pInputData, BcU32 InputSize, const BcU8*& pOutputData, BcU32& OutputSize )
{
	uLong OutSize = compressBound( InputSize );
	BcU8* pOutData = new BcU8[ OutSize ];

	int RetVal = compress( pOutData, &OutSize, pInputData, InputSize );

	if( RetVal == Z_OK && OutSize < InputSize )
	{
		pOutputData = pOutData;
		OutputSize = OutSize;
		return BcTrue;
	}

	delete [] pOutData;
	pOutData = NULL;
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// BcDecompressData
BcBool BcDecompressData( const BcU8* pInputData, BcU32 InputSize, BcU8* pOutputData, BcU32 OutputSize )
{
	uLongf DestLen = OutputSize;

	int RetVal = uncompress( pOutputData, &DestLen, pInputData, InputSize );

	if( RetVal == Z_OK )
	{
		return BcTrue;
	}

	return BcFalse;
}
