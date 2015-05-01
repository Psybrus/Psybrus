#pragma once

#include "Reflection/ReReflection.h"

#include "System/Debug/DsImGui.h"

//////////////////////////////////////////////////////////////////////////
// DsImGuiFieldEditor
class DsImGuiFieldEditor:
	public ReAttribute
{
public:
	using CustomFunc = std::function< void( DsImGuiFieldEditor*, std::string Name, void*, const ReClass*, ReFieldFlags ) >;

public:
REFLECTION_DECLARE_DERIVED( DsImGuiFieldEditor, ReAttribute );

public:
	DsImGuiFieldEditor( CustomFunc CustomFunc = nullptr );
	virtual ~DsImGuiFieldEditor();

	virtual void onEdit( std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags );

	/**
	 * Get editor for field.
	 */
	static DsImGuiFieldEditor* Get( const ReField* Field );

	/**
	 * Get editor for class.
	 */
	static DsImGuiFieldEditor* Get( const ReClass* Class );

private:
	CustomFunc CustomFunc_;
};
