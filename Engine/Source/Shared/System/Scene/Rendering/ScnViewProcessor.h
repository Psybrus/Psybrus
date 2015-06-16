#pragma once

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCoreCallback.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewProcessFunc
using ScnViewProcessFunc = 
	std::function< void( 
		const ScnComponentList&,
		class ScnViewComponent*, 
		class RsFrame*,
		RsRenderSort ) >;

struct ScnViewProcessFuncEntry
{
	const ReClass* Class_;
	std::string Name_;
	ScnViewProcessFunc Func_;

	ScnViewProcessFuncEntry(
			std::string Name,
			ScnViewProcessFunc Func ):
		Class_( nullptr ),
		Name_( Name ),
		Func_( Func )
	{}

	/**
	 * Backwards compatibility: render support.
	 */
	template< class _Ty >
	static ScnViewProcessFuncEntry Render(
		std::string Name = "Render" )
	{
		return ScnViewProcessFuncEntry( Name,
			[]( const ScnComponentList& InComponents,
				class ScnViewComponent* View, 
				class RsFrame* Frame,
				RsRenderSort Sort )
			{
				for( auto Component : InComponents )
				{
					BcAssert( Component->isTypeOf< _Ty >() );
					auto* CastComponent = static_cast< _Ty* >( Component.get() );
					CastComponent->render( View, Frame, Sort );
				}
			} );
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnViewProcessor
class ScnViewProcessor:
	public ScnComponentProcessor,
	public ScnCoreCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnViewProcessor, ScnComponentProcessor );

	ScnViewProcessor();
	virtual ~ScnViewProcessor();

	void initialise() override;
	void shutdown() override;

	void registerProcessFunc( const ReClass* Class, ScnViewProcessFuncEntry ProcessFunc );
	void deregisterProcessFunc( const ReClass* Class );

private:
	void renderViews( const ScnComponentList& InComponents );

private:
	void onAttachComponent( class ScnComponent* Component ) override;
	void onDetachComponent( class ScnComponent* Component ) override;

private:
	std::set< class ScnRenderableComponent* > RenderableComponents_;
	std::set< class ScnViewComponent* > ViewComponents_;
	std::vector< ScnViewProcessFuncEntry > ProcessFuncEntries_;

	ScnComponentList GatheredComponents_;
};
