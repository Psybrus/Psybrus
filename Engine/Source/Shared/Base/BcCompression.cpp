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
BcBool BcCompressData( const BcU8* pInputData, size_t InputSize, const BcU8*& pOutputData, size_t& OutputSize )
{
	uLong OutSize = compressBound( (uLong)InputSize );
	BcU8* pOutData = new BcU8[ OutSize ];

	int RetVal = compress( pOutData, &OutSize, pInputData, (uLong)InputSize );

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
BcBool BcDecompressData( const BcU8* pInputData, size_t InputSize, BcU8* pOutputData, size_t OutputSize )
{
	uLongf DestLen = (uLongf)OutputSize;

	int RetVal = uncompress( pOutputData, &DestLen, pInputData, (uLong)InputSize );

	if( RetVal == Z_OK )
	{
		return BcTrue;
	}

	return BcFalse;
}
