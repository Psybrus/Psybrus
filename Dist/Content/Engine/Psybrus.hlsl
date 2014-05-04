#include "PsybrusUniforms.hlsl"

#if defined( PERM_MESH_SKINNED )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be a structure containing Position_, BlendIndices_, and BlendWeights_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) 																\
		_o = float4( 0.0, 0.0, 0.0, 0.0 );																		\
		_o += mul( uBoneTransform[ int(_v.BlendIndices_.x) ], _v.Position_ ) * _v.BlendWeights_.x;				\
		_o += mul( uBoneTransform[ int(_v.BlendIndices_.y) ], _v.Position_ ) * _v.BlendWeights_.y;				\
		_o += mul( uBoneTransform[ int(_v.BlendIndices_.z) ], _v.Position_ ) * _v.BlendWeights_.z;				\
		_o += mul( uBoneTransform[ int(_v.BlendIndices_.w) ], _v.Position_ ) * _v.BlendWeights_.w;				\


#elif defined( PERM_MESH_PARTICLE )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be a structure containing Position_, VertexOffset_
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) 																\
		_o = _v.Position_ + 																					\
				float4(																							\
					mul(																						\
						float3x3(																				\
							uInverseViewTransform[0].xyz, 														\
							uInverseViewTransform[1].xyz, 														\
							uInverseViewTransform[2].xyz ),														\
			 			_v.VertexOffset_.xyz ),	0.0 );															\

#elif defined( PERM_MESH_INSTANCED )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be a structure containing Position_ and WorldMatrix[0-3].
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) 																\
		_o = mul( 																								\
				float4x4(  																						\
					_v.WorldMatrix0_, 																			\
					_v.WorldMatrix1_, 																			\
					_v.WorldMatrix2_, 																			\
					_v.WorldMatrix3_ ), _v.Position_ );	 														\

#else

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be a structure containing Position_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) 																\
		_o = mul( uWorldTransform, _v.Position_ );																\

#endif
