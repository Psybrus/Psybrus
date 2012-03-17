/**************************************************************************
*
* File:		CsFileWriter.h
* Author:	Neil Richardson 
* Ver/Date:	12/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSFILEWRITER_H__
#define __CSFILEWRITER_H__

#include "CsFile.h"

#include "BcFile.h"
#include "BcHash.h"

//////////////////////////////////////////////////////////////////////////
// CsFileWriter
class CsFileWriter:
	public CsFile
{
public:
	CsFileWriter( const std::string& Name );
	virtual ~CsFileWriter();
	
	virtual BcBool				save();
	BcU32						addString( const BcChar* pString );
	BcU32						addChunk( BcU32 ID, void* pData, BcU32 Size );
	
private:
	typedef std::vector< CsFileChunkNative > CsFileChunkNativeList;
	typedef CsFileChunkNativeList::iterator CsFileChunkNativeListIterator;

	typedef std::vector< std::string > TStringList;
	typedef TStringList::iterator TStringListIterator;
	
	BcFile						File_;
	CsFileHeader				Header_;
	TStringList					StringList_;
	CsFileChunkNativeList		Chunks_;
};


#endif
