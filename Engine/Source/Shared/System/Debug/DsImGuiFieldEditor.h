#pragma once

#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// DsImGuiFieldEditor
class DsImGuiFieldEditor:
	public ReAttribute
{
public:
	using CustomFunc = std::function< void( std::string Name, void*, const ReClass* ) >;

public:
	REFLECTION_DECLARE_DERIVED( DsImGuiFieldEditor, ReAttribute );

public:
	DsImGuiFieldEditor( CustomFunc CustomFunc = nullptr );

	virtual void onEdit( std::string Name, void* Object, const ReClass* Class );

private:
	CustomFunc CustomFunc_;
};
