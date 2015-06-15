#pragma once

#include "System/Scene/ScnTypes.h"
#include "Reflection/ReReflection.h"
#include "System/Scene/ScnComponentPriority.h"
#include "System/Scene/ScnComponent.h"

#include <initializer_list>

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessFunc
using ScnComponentProcessFunc = std::function< void( const ScnComponentList& ) >;

struct ScnComponentProcessFuncEntry
{
	ReClass* Class_;
	std::string Name_;
	ScnComponentPriority Priority_;
	ScnComponentProcessFunc Func_;

	ScnComponentProcessFuncEntry(
			std::string Name,
			ScnComponentPriority Priority,
			ScnComponentProcessFunc Func ):
		Class_( nullptr ),
		Name_( Name ),
		Priority_( Priority ),
		Func_( Func )
	{}

	/**
	 * Backwards compatibility: preUpdate support.
	 */
	template< class _Ty >
	static ScnComponentProcessFuncEntry PreUpdate(
		std::string Name = "Pre Update",
		ScnComponentPriority Priority = ScnComponentPriority::DEFAULT_PRE_UPDATE )
	{
		return ScnComponentProcessFuncEntry( Name, Priority,
			[]( const ScnComponentList& Components )
			{
				auto Tick = getTick();
				for( auto Component : Components )
				{
					BcAssert( Component->isTypeOf< _Ty >() );
					auto* CastComponent = static_cast< _Ty* >( Component.get() );
					CastComponent->preUpdate( Tick );
				}
			} );
	}

	/**
	 * Backwards compatibility: update support.
	 */
	template< class _Ty >
	static ScnComponentProcessFuncEntry Update(
		std::string Name = "Update",
		ScnComponentPriority Priority = ScnComponentPriority::DEFAULT_UPDATE )
	{
		return ScnComponentProcessFuncEntry( Name, Priority,
			[]( const ScnComponentList& Components )
			{
				auto Tick = getTick();
				for( auto Component : Components )
				{
					BcAssert( Component->isTypeOf< _Ty >() );
					auto* CastComponent = static_cast< _Ty* >( Component.get() );
					CastComponent->update( Tick );
				}
			} );
	}

	/**
	 * Backwards compatibility: postUpdate support.
	 */
	template< class _Ty >
	static ScnComponentProcessFuncEntry PostUpdate(
		std::string Name = "Post Update",
		ScnComponentPriority Priority = ScnComponentPriority::DEFAULT_POST_UPDATE )
	{
		return ScnComponentProcessFuncEntry( Name, Priority,
			[]( const ScnComponentList& Components )
			{
				auto Tick = getTick();
				for( auto Component : Components )
				{
					BcAssert( Component->isTypeOf< _Ty >() );
					auto* CastComponent = static_cast< _Ty* >( Component.get() );
					CastComponent->postUpdate( Tick );
				}
			} );
	}

private:
	static BcF32 getTick();
};

using ScnComponentProcessFuncEntryList = std::vector< ScnComponentProcessFuncEntry >;
using ScnComponentProcessFuncEntryInitialiserList = std::initializer_list< ScnComponentProcessFuncEntry >;

//////////////////////////////////////////////////////////////////////////
// ScnComponentProcessor
class ScnComponentProcessor:
	public ReAttribute
{
public:
	REFLECTION_DECLARE_DERIVED( ScnComponentProcessor, ReAttribute );

	ScnComponentProcessor();
	ScnComponentProcessor( ScnComponentProcessFuncEntryInitialiserList ProcessFuncEntries );

	/**
	 * Called when the scene system is initialised.
	 */
	virtual void initialise();
		
	/**
	 * Called when the scene system is shutdown.
	 */
	virtual void shutdown();

	/**
	 * Called to get the process funcs.
	 * By default it will register preUpdate, update, and postUpdate
	 * calls to emulate the original. Override to implement your own.
	 * @param Class Component class we should be updating.
	 */
	virtual const ScnComponentProcessFuncEntryList& getProcessFuncs();

private:
	ScnComponentProcessFuncEntryList ProcessFuncs_;
};
