/**************************************************************************
*
* File:		ScnAnimationImport.h
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnAnimationImport_H__
#define __ScnAnimationImport_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Scene/Animation/ScnAnimation.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationImport
class ScnAnimationImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnAnimationImport, CsResourceImporter );

public:
	ScnAnimationImport();
	virtual ~ScnAnimationImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	// Animated node, used in calculation
	// of local space animation stuff.
	struct AnimatedNode
	{
		std::string Name_;
		MaMat4d LocalTransform_;
		MaMat4d WorldTransform_;
		BcU32 ParentIdx_;
	};

	void recursiveParseAnimatedNodes( struct aiNode* Node, BcU32 ParentNodeIdx );
	
	AnimatedNode& findAnimatedNode( std::string Name );

private:
	std::string Source_;

	BcStream HeaderStream_;
	BcStream NodeStream_;
	BcStream PoseStream_;
	BcStream KeyStream_;
	const struct aiScene* Scene_;
	std::vector< AnimatedNode > AnimatedNodes_;

};

#endif

#endif
