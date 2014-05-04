////////////////////////////////////////////////////////////////////////
// VertexDefault
struct VertexDefault
{
	float4 Position_		: POSITION;
	float4 Colour_			: COLOR0;
	float4 Tangent_			: TANGENT;

#if defined( PERM_MESH_SKINNED )
	float4 BlendIndices_	: BLENDINDICES;
	float4 BlendWeights_	: BLENDWEIGHTS;

#elif defined( PERM_MESH_PARTICLE )
	float4 VertexOffset_	: NORMAL;

#elif defined( PERM_MESH_PARTICLE )
	float4 WorldMatrix0_	: TEXCOORD0;
	float4 WorldMatrix1_	: TEXCOORD1;
	float4 WorldMatrix2_	: TEXCOORD2;
	float4 WorldMatrix3_	: TEXCOORD3;
#endif
};

////////////////////////////////////////////////////////////////////////
// PSY_MAKE_WORLD_SPACE_VERTEX
#if defined( PERM_MESH_STATIC )
/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Unused.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = mul( uWorldTransform, _v );																\


#elif defined( PERM_MESH_SKINNED )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing BlendIndices_, and BlendWeights_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 																\
		_o = float4( 0.0, 0.0, 0.0, 0.0 );																\
		_o += mul( uBoneTransform[ int(_p.BlendIndices_.x) ], _v ) * _p.BlendWeights_.x;				\
		_o += mul( uBoneTransform[ int(_p.BlendIndices_.y) ], _v ) * _p.BlendWeights_.y;				\
		_o += mul( uBoneTransform[ int(_p.BlendIndices_.z) ], _v ) * _p.BlendWeights_.z;				\
		_o += mul( uBoneTransform[ int(_p.BlendIndices_.w) ], _v ) * _p.BlendWeights_.w;				\


#elif defined( PERM_MESH_PARTICLE )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing VertexOffset_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 																\
		_o = _v + 																						\
				float4(																					\
					mul(																				\
						float3x3(																		\
							uInverseViewTransform[0].xyz, 												\
							uInverseViewTransform[1].xyz, 												\
							uInverseViewTransform[2].xyz ),												\
			 			_p.VertexOffset_.xyz ),	0.0 );													\

#elif defined( PERM_MESH_INSTANCED )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing WorldMatrix[0-3].
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = mul( 																						\
				float4x4(  																				\
					_p.WorldMatrix0_, 																	\
					_p.WorldMatrix1_, 																	\
					_p.WorldMatrix2_, 																	\
					_p.WorldMatrix3_ ), _v );	 														\

#endif

