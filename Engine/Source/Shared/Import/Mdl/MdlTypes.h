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

#include "Base/BcTypes.h"
#include "Base/BcVec2d.h"
#include "Base/BcVec3d.h"
#include "Base/BcQuat.h"
#include "Base/BcPlane.h"
#include "Base/BcAABB.h"
#include "Base/BcMemory.h"

#include <vector>
#include <string>

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
	BcF32		SpecularPower_;

	eBlendMode	BlendMode_;

	BcBool		AlphaTest_;
	BcF32		AlphaTestThreshold_;
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
	{
		BcMemSet( &iJoints_, 0xff, sizeof( iJoints_ ) );
		BcMemSet( &Weights_, 0, sizeof( Weights_ ) );
	}

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
	BcF32 Weights_[ 4 ];
	BcU32 nWeights_;
};

struct MdlAnimKey
{
	BcMat4d Matrix_;
};

struct MdlAnimNode
{
	BcChar					Name_[ MDL_NODENAME_LEN ];
	BcChar					Parent_[ MDL_NODENAME_LEN ];
	std::vector< MdlAnimKey > KeyList_;
};

struct MdlTriangle
{
	MdlVertex Vertex_[ 3 ];
};

struct MdlBonePalette
{
	std::vector< BcU32 > BonePalette_;

	MdlBonePalette( BcU32 Size = 24 )
	{
		BonePalette_.resize( Size );
		for( BcU32 Idx = 0; Idx < Size; ++Idx )
		{
			BonePalette_[ Idx ] = BcErrorCode;
		}
	}
	
	BcBool haveJoint( BcU32 JointIdx ) const
	{
		for( BcU32 Idx = 0; Idx < BonePalette_.size(); ++Idx )
		{
			const BcU32& BoneIdx = BonePalette_[ Idx ];
			if( BoneIdx == JointIdx )
			{
				return BcTrue;
			}
		}

		return BcFalse;
	}

	BcU32 addJoint( BcU32 JointIdx )
	{
		for( BcU32 Idx = 0; Idx < BonePalette_.size(); ++Idx )
		{
			BcU32& BoneIdx = BonePalette_[ Idx ];
			if( BoneIdx == BcErrorCode ||
				BoneIdx == JointIdx )
			{
				BoneIdx = JointIdx;
				return Idx;
			}
		}

		return BcErrorCode;
	}

	BcU32 freeBonePaletteEntries() const
	{
		BcU32 TotalFree = 0;
		for( BcU32 Idx = 0; Idx < BonePalette_.size(); ++Idx )
		{
			if( BonePalette_[ Idx ] == BcErrorCode )
			{
				++TotalFree;
			}
		}
		return TotalFree;
	}

	BcBool addTriangle( MdlTriangle& Triangle )
	{
		BcU32 FreeBonePaletteEntries = freeBonePaletteEntries();
		for( BcU32 TriIdx = 0; TriIdx < 3; ++TriIdx )
		{
			MdlVertex& Vertex = Triangle.Vertex_[ TriIdx ];
			for( BcU32 JointIdx = 0; JointIdx < 4; ++JointIdx )
			{
				BcF32 Weight = Vertex.Weights_[ JointIdx ];
				BcU32 Joint = Vertex.iJoints_[ JointIdx ];
				if( Weight > 0.0f && Joint != BcErrorCode )
				{
					if( haveJoint( Joint ) == BcFalse )
					{
						if( FreeBonePaletteEntries > 0 )
						{
							--FreeBonePaletteEntries;
						}
						else
						{
							return BcFalse;
						}
					}
				}
			}
		}

		for( BcU32 TriIdx = 0; TriIdx < 3; ++TriIdx )
		{
			MdlVertex& Vertex = Triangle.Vertex_[ TriIdx ];
			for( BcU32 JointIdx = 0; JointIdx < 4; ++JointIdx )
			{
				BcF32& Weight = Vertex.Weights_[ JointIdx ];
				BcU32& Joint = Vertex.iJoints_[ JointIdx ];
				if( Weight > 0.0f && Joint != BcErrorCode )
				{
					Joint = addJoint( Joint );
					BcAssert( Joint != BcErrorCode );
				}
				else
				{
					Weight = 0.0f;
					Joint = BcErrorCode;
				}
			}
		}

		return BcTrue;
	}
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
	BcF32 AttnC_;
	BcF32 AttnL_;
	BcF32 AttnQ_;
};

struct MdlProjector
{
	BcF32 Fov_;
	BcF32 Aspect_;
	BcF32 ZNear_;
	BcF32 ZFar_;
};

struct MdlBspNode;
typedef std::vector< MdlBspNode* > MdlBspNodeList;

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
typedef std::vector< MdlTriangle >		MdlTriangleArray;
typedef std::vector< MdlVertex >		MdlVertexArray;
typedef std::vector< MdlMaterial >		MdlMaterialArray;
typedef std::vector< MdlEntityProp >	MdlEntityPropArray;

#endif
