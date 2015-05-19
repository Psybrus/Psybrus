#pragma once

#include "System/Scene/ScnTypes.h"
#include "Reflection/ReReflection.h"
#include "System/Scene/ScnComponentPriority.h"

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

	ScnComponentProcessor( BcS32 Priority = 0 );

	ScnComponentProcessor( ScnComponentProcessFuncEntryInitialiserList ProcessFuncEntries );

	/**
	 * Called to get the process funcs.
	 * By default it will register preUpdate, update, and postUpdate
	 * calls to emulate the original. Override to implement your own.
	 * @param Class Component class we should be updating.
	 */
	virtual const ScnComponentProcessFuncEntryList& getProcessFuncs();


private:
	BcS32 Priority_;
	ScnComponentProcessFuncEntryList ProcessFuncs_;
};
