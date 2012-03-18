/**************************************************************************
*
* File:		MD5AnimLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __MD5AnimLoader_H__
#define __MD5AnimLoader_H__

#include "Base/BcVectors.h"
#include "Base/BcQuat.h"

#include "MD5MeshLoader.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MdlAnim;

//////////////////////////////////////////////////////////////////////////
// Structures
struct MD5_Bound
{
	BcF32 MinX_, MinY_, MinZ_;
	BcF32 MaxX_, MaxY_, MaxZ_;
};

struct MD5_Frame
{
	BcU32 nKeys_;
	MD5_Joint* pKeys_;
};

//////////////////////////////////////////////////////////////////////////
// MD5AnimLoader
class MD5AnimLoader
{
public:
	MD5AnimLoader();
	~MD5AnimLoader();

	MdlAnim* load( const BcChar* FileName, const BcChar* NodeName );

private:
	BcU32 nJoints() const;
	MD5_Joint* pJoint( BcU32 Index );

	BcU32 nFrames() const;
	MD5_Frame* pFrame( BcU32 Index );

private:
	enum eParseMode
	{
		PM_MAIN = 0,
		PM_HIERARCHY,
		PM_BOUNDS,
		PM_BASEFRAME,
		PM_FRAME,
	};

	eParseMode ParseMode_;

	BcU32 nJoints_;
	MD5_Joint* pJoints_;

	BcU32 nFrames_;
	MD5_Frame* pFrames_;

	BcU32 nBounds_;
	MD5_Bound* pBounds_;

	BcF32 FrameRate_;
	
	BcU32 nAnimComponents_;
	BcF32* pAnimComponents_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcU32 MD5AnimLoader::nJoints() const
{
	return nJoints_;
}

inline MD5_Joint* MD5AnimLoader::pJoint( BcU32 Index )
{
	return &pJoints_[ Index ];
}

inline BcU32 MD5AnimLoader::nFrames() const
{
	return nFrames_;
}

inline MD5_Frame* MD5AnimLoader::pFrame( BcU32 Index )
{
	return &pFrames_[ Index ];
}

#endif
