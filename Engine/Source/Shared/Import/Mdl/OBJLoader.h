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

#include "Base/BcVectors.h"
#include "Base/BcQuat.h"
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
	std::vector< BcVec3d > Positions_;
	std::vector< BcVec3d > Normals_;
	std::vector< BcVec2d > TexCoords_;

};



#endif
