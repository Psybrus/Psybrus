/**************************************************************************
*
* File:		BcCompression.h
* Author: 	Neil Richardson 
* Ver/Date:	9/04/2012
* Description:
*		
*		
*		
* 
**************************************************************************/

#ifndef __BCCOMPRESSION_H__
#define __BCCOMPRESSION_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
/* @class BcCompressData
 *
 * Compresses data into an output buffer. Will allocate the output buffer for you.
 */
extern BcBool BcCompressData( const BcU8* pInputData, size_t InputSize, const BcU8*& pOutputData, size_t& OutputSize );

//////////////////////////////////////////////////////////////////////////
/* @class BcDecompressData
 * 
 * Decompresses data into the output buffer. Does not allocate the output buffer.
 */
extern BcBool BcDecompressData( const BcU8* pInputData, size_t InputSize, BcU8* pOutputData, size_t OutputSize );


#endif