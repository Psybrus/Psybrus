/**************************************************************************
*
* File:		MdlTypes.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Fundamental data types for models.
*		
*
*
* 
**************************************************************************/

#ifndef __MDLTYPES_H__
#define __MDLTYPES_H__

#include "BcTypes.h"
#include "BcVectors.h"
#include "BcQuat.h"
#include "BcPlane.h"
#include "BcAABB.h"

//////////////////////////////////////////////////////////////////////////
// Defines

#define MDL_NODENAME_LEN		128
#define MDL_MAXPROP_LEN			128
#define MDL_DEFAULT_NODE_NAME	"UNNAMED"
#define MDL_MAXBSP_VERTS		16

//////////////////////////////////////////////////////////////////////////
// Enumerations
enum
{
	eNT_EMPTY			=	0x0000,
	eNT_MESH			=	0x0001,
	eNT_SKIN			=	0x0002,
	eNT_MORPH			=	0x0004,
	eNT_ENTITY			=	0x0008,
	eNT_COLMESH			=	0x0010,
	eNT_JOINT			=	0x0020,
	eNT_LIGHT			=	0x0040,
	eNT_PROJECTOR		=	0x0080,
	eNT_SOUNDEMITTER	=	0x0100,
};

//////////////////////////////////////////////////////////////////////////
// Fundamental types

typedef BcVec3d			MdlPosition;
typedef BcVec3d			MdlNormal;
typedef BcVec3d			MdlTangent;
typedef BcVec2d			MdlUV;
typedef BcVec4d			MdlColour;
typedef BcU32			MdlMaterialIndex;

class MdlMaterial
{
public:
	enum eBlendMode
	{
		BM_NONE,
		BM_BLEND,
		BM_ADDITIVE,
		BM_SUBTRACTIVE,
	};
	
	void default2d()
	{
		AmbientColour_.set( 0.0f, 0.0f, 0.0f, 1.0f );
		DiffuseColour_.set( 1.0f, 1.0f, 1.0f, 1.0f );
		SpecularColour_.set( 0.0f, 0.0f, 0.0f, 1.0f );
		SpecularPower_ = 0.0f;
		BlendMode_ = BM_BLEND;
		AlphaTest_ = BcFalse;
		AlphaTestThreshold_ = 0.0f;
		DepthTest_ = BcFalse;
		DepthWrite_ = BcFalse;
		Lit_ = BcFalse;
		Layer_ = 0;
	}
	
	void default3d()
	{
		AmbientColour_.set( 0.0f, 0.0f, 0.0f, 1.0f );
		DiffuseColour_.set( 1.0f, 1.0f, 1.0f, 1.0f );
		SpecularColour_.set( 0.0f, 0.0f, 0.0f, 1.0f );
		SpecularPower_ = 1.0f;
		BlendMode_ = BM_NONE;
		AlphaTest_ = BcFalse;
		AlphaTestThreshold_ = 0.0f;
		DepthTest_ = BcTrue;
		DepthWrite_ = BcTrue;
		Lit_ = BcFalse;
		Layer_ = 0;
	}
	
	BcBool operator == ( const MdlMaterial& Other ) const
	{
		return ( Other.Name_ == Name_ );
	}

	std::string	Name_;

	MdlColour	AmbientColour_;
	MdlColour	DiffuseColour_;
	MdlColour	SpecularColour_;
	BcReal		SpecularPower_;

	eBlendMode	BlendMode_;

	BcBool		AlphaTest_;
	BcReal		AlphaTestThreshold_;
	BcBool		DepthTest_;
	BcBool		DepthWrite_;
	BcBool		Lit_;
	BcU32		Layer_;
};

struct MdlIndex
{
	BcU32 iVertex_;
	BcU32 iMaterial_;
};

//////////////////////////////////////////////////////////////////////////
// Useful structures

struct MdlVertex
{
	MdlVertex():
		Position_( MdlPosition( 0.0f, 0.0f, 0.0f ) ),
		bPosition_( BcFalse ),
		Normal_( MdlNormal( 0.0f, 0.0f, 0.0f ) ),
		bNormal_( BcFalse ),
		Tangent_( MdlTangent( 0.0f, 0.0f, 0.0f ) ),
		bTangent_( BcFalse ),
		UV_( MdlUV( 0.0f, 0.0f ) ),
		bUV_( BcFalse ),
		Colour_( MdlColour( 1.0f, 1.0f, 1.0f, 1.0f ) ),
		bColour_( BcFalse ),
		nWeights_( 0 )
	{}

	MdlPosition Position_;
	BcBool bPosition_;

	MdlNormal Normal_;
	BcBool bNormal_;

	MdlTangent Tangent_;
	BcBool bTangent_;

	MdlUV UV_;
	BcBool bUV_;

	MdlColour Colour_;
	BcBool bColour_;

	BcU32 iJoints_[ 4 ];
	BcReal Weights_[ 4 ];
	BcU32 nWeights_;
};

struct MdlAnimKey
{
	BcQuat	R_;
	BcVec3d S_;
	BcVec3d T_;
};

struct MdlAnimNode
{
	BcChar					Name_[ MDL_NODENAME_LEN ];
	BcChar					Parent_[ MDL_NODENAME_LEN ];
	std::vector< MdlAnimKey > KeyList_;
};

class MdlLight
{
public:
	enum eType
	{
		T_OMNI = 0,
		T_DIRECTIONAL,
		T_SPOT
	};
public:
	eType Type_;
	MdlColour Colour_;
	BcReal AttnC_;
	BcReal AttnL_;
	BcReal AttnQ_;
};

struct MdlProjector
{
	BcReal Fov_;
	BcReal Aspect_;
	BcReal ZNear_;
	BcReal ZFar_;
};

struct MdlBspNode;
typedef std::list< MdlBspNode* > MdlBspNodeList;

struct MdlBspNode
{
	MdlBspNode():
		Index_( BcErrorCode ),
		pFront_( NULL ),
		pBack_( NULL ),
		nVertices_( 0 )
	{}

	BcU32 Index_;
	MdlBspNode* pFront_;
	MdlBspNode* pBack_;

	BcPlane Plane_;
	BcU32 nVertices_;
	BcVec3d Vertices_[ MDL_MAXBSP_VERTS ];

	MdlBspNodeList FrontList_;		// Infront of us
	MdlBspNodeList BackList_;		// Behind us.
};


//////////////////////////////////////////////////////////////////////////
// Entities

struct MdlEntityProp
{
	BcChar Property_[ MDL_MAXPROP_LEN ];
	BcChar Value_[ MDL_MAXPROP_LEN ];
};


//////////////////////////////////////////////////////////////////////////
// Utility

typedef std::vector< MdlIndex >			MdlIndexArray;
typedef std::vector< MdlVertex >		MdlVertexArray;
typedef std::vector< MdlMaterial >		MdlMaterialArray;
typedef std::vector< MdlEntityProp >	MdlEntityPropArray;

#endif
