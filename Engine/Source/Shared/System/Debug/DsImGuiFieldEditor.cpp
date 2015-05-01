#include "System/Debug/DsImGuiFieldEditor.h"

//////////////////////////////////////////////////////////////////////////
// DsImGuiFieldEditor
REFLECTION_DEFINE_DERIVED( DsImGuiFieldEditor );

void DsImGuiFieldEditor::StaticRegisterClass()
{
	ReRegisterClass< DsImGuiFieldEditor, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
DsImGuiFieldEditor::DsImGuiFieldEditor( CustomFunc CustomFunc ):
	CustomFunc_( CustomFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsImGuiFieldEditor::~DsImGuiFieldEditor()
{

}

//////////////////////////////////////////////////////////////////////////
// onEdit
//virtual
void DsImGuiFieldEditor::onEdit( std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
{
	if( CustomFunc_ )
	{
		CustomFunc_( this, Name, Object, Class, Flags );
	}
}

//////////////////////////////////////////////////////////////////////////
// Get
//static
DsImGuiFieldEditor* DsImGuiFieldEditor::Get( const ReField* Field )
{
	DsImGuiFieldEditor* FieldEditor = Field->getAttribute< DsImGuiFieldEditor >();
	if( FieldEditor == nullptr )
	{
		FieldEditor = Get( Field->getType() );
	}
	return FieldEditor;
}

//////////////////////////////////////////////////////////////////////////
// Get
//static
DsImGuiFieldEditor* DsImGuiFieldEditor::Get( const ReClass* Class )
{
	DsImGuiFieldEditor* FieldEditor = nullptr;
	while( FieldEditor == nullptr && Class != nullptr )
	{
		FieldEditor = Class->getAttribute< DsImGuiFieldEditor >();
		Class = Class->getSuper();
	}
	return FieldEditor;
}
