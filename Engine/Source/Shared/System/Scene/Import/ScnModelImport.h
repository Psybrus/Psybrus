/**************************************************************************
*
* File:		ScnModelImport.h
* Author:	Neil Richardson 
* Ver/Date: 06/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODELIMPORT_H__
#define __SCNMODELIMPORT_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Scene/ScnModelFileData.h"

#include "Base/BcStream.h"
#include "Import/Mdl/Mdl.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelImport
class ScnModelImport
{
public:
	ScnModelImport();

	/**
	 * Import.
	 */
	BcBool import( class CsPackageImporter& Importer, const Json::Value& Object );

private:
	void								recursiveSerialiseNodes( class MdlNode* pNode,
																 BcU32 ParentIndex,
																 BcU32& NodeIndex,
																 BcU32& PrimitiveIndex );

private:
	std::string							Source_;

	CsPackageImporter*					pImporter_;
	BcStream							HeaderStream_;
	BcStream							NodeTransformDataStream_;
	BcStream							NodePropertyDataStream_;
	BcStream							VertexDataStream_;
	BcStream							IndexDataStream_;
	BcStream							PrimitiveDataStream_;

};

#endif

#endif
