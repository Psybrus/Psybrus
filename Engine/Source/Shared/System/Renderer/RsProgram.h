/**************************************************************************
*
* File:		RsProgram.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAM_H__
#define __RSPROGRAM_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

////////////////////////////////////////////////////////////////////////////////
// RsProgram
class RsProgram:
	public RsResource
{
public:
	RsProgram( 
		class RsContext* pContext, 
		std::vector< class RsShader* >&& Shaders,
		RsProgramVertexAttributeList&& VertexAttributes,
		RsProgramParameterList&& ParameterList );
	virtual ~RsProgram();
	
	BcU32 findSamplerSlot( const BcChar* Name ) const;
	BcU32 findShaderResourceSlot( const BcChar* Name ) const;
	BcU32 findUnorderedAccessSlot( const BcChar* Name ) const;
	BcU32 findUniformBufferSlot( const BcChar* Name ) const;

	const char* getSamplerSlotName( BcU32 Slot ) const;
	const char* getShaderResourceSlotName( BcU32 Slot ) const;
	const char* getUnorderedAccessSlotName( BcU32 Slot ) const;
	const char* getUniformBufferSlotName( BcU32 Slot ) const;

	const ReClass* getUniformBufferClass( BcU32 Handle );

	const std::vector< class RsShader* >& getShaders() const;
	const RsProgramVertexAttributeList& getVertexAttributeList() const;
	const RsProgramParameterList& getParameterList() const;
	BcU32 getInputLayoutHash() const;

	bool isGraphics() const;
	bool isCompute() const;



public:
	// Used internally by the renderer to patch reflection information
	// into the program.
	void addSamplerSlot( std::string Name, BcU32 Handle );
	void addShaderResource( std::string Name, RsShaderResourceType Type, BcU32 Handle );
	void addUnorderedAccess( std::string Name, RsUnorderedAccessType Type, BcU32 Handle );
	void addUniformBufferSlot( std::string Name, BcU32 Handle, const ReClass* Class );

private:
	struct TSampler
	{
		std::string Name_;
		BcU32 Handle_;
	};

	struct TShaderResource
	{
		std::string Name_;
		RsShaderResourceType Type_;
		BcU32 Handle_;
	};

	struct TUnorderedAccess
	{
		std::string Name_;
		RsUnorderedAccessType Type_;
		BcU32 Handle_;
	};

	struct TUniformBlock
	{
		std::string Name_;
		BcU32 Handle_;
		const ReClass* Class_;
	};

	typedef std::vector< TSampler > TSamplerList;
	TSamplerList SamplerList_;

	typedef std::vector< TShaderResource > TShaderResourceList;
	TShaderResourceList ShaderResourceList_;

	typedef std::vector< TUnorderedAccess > TUnorderedAccessList;
	TUnorderedAccessList UnorderedAccessList_;

	typedef std::vector< TUniformBlock > TUniformBlockList;
	TUniformBlockList InternalUniformBlockList_;
	
	std::vector< class RsShader* > Shaders_;
	RsProgramVertexAttributeList AttributeList_;
	RsProgramParameterList ParameterList_;
	BcU32 InputLayoutHash_;
};

#endif
