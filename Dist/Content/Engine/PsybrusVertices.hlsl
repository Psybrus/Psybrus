////////////////////////////////////////////////////////////////////////
// VertexDefault
struct VertexDefault
{
	float4 Position_		: POSITION;
	float4 Normal_			: NORMAL;
	float4 Colour_			: COLOR0;
	float4 TexCoord0_		: TEXCOORD0;

#if defined( PERM_MESH_SKINNED_3D )
	float4 BlendIndices_	: BLENDINDICES;
	float4 BlendWeights_	: BLENDWEIGHTS;

#elif defined( PERM_MESH_PARTICLE_3D )
	float4 VertexOffset_	: TANGENT;

#elif defined( PERM_MESH_INSTANCED_3D )
	float4 WorldMatrix0_	: TEXCOORD4;
	float4 WorldMatrix1_	: TEXCOORD5;
	float4 WorldMatrix2_	: TEXCOORD6;
	float4 WorldMatrix3_	: TEXCOORD7;
#endif
};

////////////////////////////////////////////////////////////////////////
// PSY_MAKE_WORLD_SPACE_VERTEX
#if defined( PERM_MESH_STATIC_2D )
/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Unused.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = float4( _v.xy, 0.0, 1.0 );																	\

#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = float3( 0.0f, 0.0f, 1.0 );																	\

#elif defined( PERM_MESH_STATIC_3D )
/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Unused.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = PsyMatMul( WorldTransform_, _v ); \

#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = PsyMatMul( (float3x3)NormalTransform_.xyz, _v ); \


#elif defined( PERM_MESH_SKINNED_3D )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing BlendIndices_, and BlendWeights_.
 */
#if PSY_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES

#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = _v; // TODO: Implement skinning :<

#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = _v; // TODO: Implement skinning :<

#else

#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = PsyMatMul( 																				\
			BoneTransform_[ (int)_p.BlendIndices_.x ], _v ) * _p.BlendWeights_.x;						\
		_o += PsyMatMul( 																				\
			BoneTransform_[ (int)_p.BlendIndices_.y ], _v ) * _p.BlendWeights_.y;						\
		_o += PsyMatMul( 																				\
			BoneTransform_[ (int)_p.BlendIndices_.z ], _v ) * _p.BlendWeights_.z;						\
		_o += PsyMatMul(																				\
			BoneTransform_[ (int)_p.BlendIndices_.w ], _v ) * _p.BlendWeights_.w;						\
			

#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = PsyMatMul( 																				\
			(float3x3)BoneTransform_[ (int)_p.BlendIndices_.x ], _v ) * _p.BlendWeights_.x;				\
		_o += PsyMatMul( 																				\
			(float3x3)BoneTransform_[ (int)_p.BlendIndices_.y ], _v ) * _p.BlendWeights_.y;				\
		_o += PsyMatMul( 																				\
			(float3x3)BoneTransform_[ (int)_p.BlendIndices_.z ], _v ) * _p.BlendWeights_.z;				\

#endif // PSY_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES


#elif defined( PERM_MESH_PARTICLE_3D )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing VertexOffset_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = _v + 																						\
				float4(																					\
					PsyMatMul(																			\
						(float3x3)InverseViewTransform_, 												\
			 			_p.VertexOffset_.xyz ),	0.0 );													\


#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = _v;																						\


#elif defined( PERM_MESH_INSTANCED_3D )

/**
 * Make a world space vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 * @param _p Input properties. Should be a structure containing WorldMatrix[0-3]_.
 */
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v, _p ) 													\
		_o = PsyMatMul( 																				\
				_p.WorldMatrix0_, 																		\
				_p.WorldMatrix1_, 																		\
				_p.WorldMatrix2_, 																		\
				_p.WorldMatrix3_, _v );	 																\


#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v, _p ) 													\
		_o = PsyMatMul( 																				\
				_p.WorldMatrix0_.xyz,																	\
				_p.WorldMatrix1_.xyz,																	\
				_p.WorldMatrix2_.xyz, _v );		 														\

#endif

////////////////////////////////////////////////////////////////////////
// PSY_MAKE_CLIP_SPACE_VERTEX
#if defined( PERM_MESH_STATIC_2D )
/**
 * Make a clip transformed vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 */
#  define PSY_MAKE_CLIP_SPACE_VERTEX( _o, _v ) 															\
		_o = _v; 																						\

#else

/**
 * Make a clip transformed vertex.
 * @param _o Output vertex. Should be float4.
 * @param _v Input vertex. Should be float4.
 */
#  define PSY_MAKE_CLIP_SPACE_VERTEX( _o, _v ) 															\
		_o = PsyMatMul( ClipTransform_, _v ); 															\

#endif