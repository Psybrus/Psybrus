/**************************************************************************
*
* File:		FBXLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		FBX Loader.
*		
*
*
* 
**************************************************************************/

#ifndef __FBXLOADER_H__
#define __FBXLOADER_H__

//////////////////////////////////////////////////////////////////////////
// FBX Includes and Defines.
#include "librarychooser.h"

#define KFBX_NODLL
#define KFBX_SDK

#include <fbxsdk.h>

#include "BcTypes.h"
#include "MdlNode.h"

//////////////////////////////////////////////////////////////////////////
// FBXLoader
class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();

	MdlNode* load( const BcChar* FileName, const BcChar* RootName );

	MdlNode* exportNode( KFbxNode* pFBXNode );

	void processMesh( KFbxNode* pFBXNode, MdlNode* pExportNode );
	void processLight( KFbxNode* pFBXNode, MdlNode* pExportNode );

private:

};

#endif
