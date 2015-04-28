#include "System/Debug/DsImGuiFieldEditor.h"

//////////////////////////////////////////////////////////////////////////
// DsImGuiFieldEditor
REFLECTION_DEFINE_DERIVED( DsImGuiFieldEditor );

void DsImGuiFieldEditor::StaticRegisterClass()
{
	ReRegisterClass< DsImGuiFieldEditor, Super >();
}

DsImGuiFieldEditor::DsImGuiFieldEditor( CustomFunc CustomFunc ):
	CustomFunc_( CustomFunc )
{
}

//virtual
void DsImGuiFieldEditor::onEdit( std::string Name, void* Object, const ReClass* Class )
{
	if( CustomFunc_ )
	{
		CustomFunc_( Name, Object, Class );
	}
}
