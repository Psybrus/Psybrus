/**************************************************************************
*
* File:		OBJLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OBJLOADER_H__
#define __OBJLOADER_H__

#include "Math/MaQuat.h"
#include "Mdl.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MdlNode;
class MdlMesh;

//////////////////////////////////////////////////////////////////////////
// OBJLoader
class OBJLoader
{
public:
	OBJLoader();
	~OBJLoader();

	MdlNode* load( const BcChar* FileName, const BcChar* NodeName );

private:
	std::vector< MaVec3d > Positions_;
	std::vector< MaVec3d > Normals_;
	std::vector< MaVec2d > TexCoords_;

};



#endif
