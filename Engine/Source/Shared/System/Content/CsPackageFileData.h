/**************************************************************************
*
* File:		CsPackageFileData.h
* Author:	Neil Richardson 
* Ver/Date:	8/04/12
* Description:
*		Package file data.
*		
*
*
* 
**************************************************************************/

#ifndef __CSPACKAGEFILEDATA_H__
#define __CSPACKAGEFILEDATA_H__

#include "Base/BcTypes.h"
#include "Base/BcAtomic.h"

//////////////////////////////////////////////////////////////////////////
/* CsPackage File Layout
*
* [CsPackageHeader] Header_;
* [BcChar] * Header_.StringTableBytes_;
* [CsPackageResourceHeader] * Header_.TotalResources_;
* [CsPackageChunkHeader] * Header_.TotalChunks_;
* [Raw chunk data]
* 
*/

//////////////////////////////////////////////////////////////////////////
// CsPackageHeader
struct CsPackageHeader
{
	static const BcU32 MAGIC = 0x89273491;

	BcU32								Magic_;					// Magic number.
	BcU32								StringTableBytes_;		// Number of bytes in the string table.
	BcU32								TotalResources_;		// Total Resources in package.
	BcU32								TotalChunks_;			// Total chunks in package.
	BcU32								SourceFile_;			// Path to source package (index into string table).
	BcU32								TotalAllocSize_;		// Total size package need to allocate (string table, resource headers, chunk headers, managed chunk data).
	BcU32								MinAlignment_;			// Minimum alignment (default 16)
	BcU32								MaxAlignment_;			// Maximum alignment (default 4096)
	BcU32								ResourceDataStart_;		// Start of resource data as an offset to allocated package memory.
};

//////////////////////////////////////////////////////////////////////////
// CsPackageResourceFlags
enum CsPackageResourceFlags
{
	csPEF_CONTIGUOUS_CHUNKS =			1 << 0,					// All Resources chunks should be contiguous in memory.
};

//////////////////////////////////////////////////////////////////////////
// CsPackageResourceHeader
struct CsPackageResourceHeader
{
	BcU32								Name_;					// Name of Resource (index into string table).
	BcU32								Type_;					// Resource type of Resource (index into string table).
	BcU32								Flags_;					// CsPackageResourceFlags.
	BcU32								FirstChunk_;			// First chunk that belongs to this resource.
	BcU32								LastChunk_;				// Last chunk that belongs to this resource.
};

//////////////////////////////////////////////////////////////////////////
// CsPackageChunkFlags
enum CsPackageChunkFlags
{
	csPCF_COMPRESSED =					1 << 0,					// Is the chunk compressed?
	csPCF_MANAGED =						1 << 1,					// Is the memory managed by the package?

	//
	csPCF_DEFAULT =						csPCF_COMPRESSED | csPCF_MANAGED,
	csPCF_IN_PLACE =					0x0						// Unmanaged and loaded in place.
};

//////////////////////////////////////////////////////////////////////////
// CsPackageChunkHeader
struct CsPackageChunkHeader
{
	BcU32								ID_;					// ID of chunk (typically a hash of a string, but can be anything).
	BcU32								Offset_;				// Offset of chunk in file.
	BcU32								Flags_;					// Flags for this chunk.
	BcU32								RequiredAlignment_;		// Required alignment.
	BcU32								PackedBytes_;			// Packed bytes in file.
	BcU32								UnpackedBytes_;			// Unpacked bytes in memory (same as PackedBytes for uncompressed data).
	BcU32								PackedHash_;			// Hash of packed data.
	BcU32								UnpackedHash_;			// Hash of unpacked data.
};

//////////////////////////////////////////////////////////////////////////
// CsPackageChunkStatus
enum CsPackageChunkStatus
{
	csPCS_NOT_LOADED = 0,
	csPCS_LOADING,
	csPCS_UNPACKING,
	csPCS_READY
};

//////////////////////////////////////////////////////////////////////////
// CsPackageChunkData
struct CsPackageChunkData
{
	CsPackageChunkStatus				Status_;				// Loading status.
	BcBool								Managed_;				// Is our memory managed?
	BcU8*								pPackedData_;			// Packed data.
	BcU8*								pUnpackedData_;			// Unpacked data.
};

#endif
