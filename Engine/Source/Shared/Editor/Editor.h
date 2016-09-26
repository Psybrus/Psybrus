#pragma once

#include "Base/BcTypes.h"
#include "Math/MaMat4d.h"

class DsImGuiFieldEditor;

namespace Editor
{
	void Init();

	/**
	 * Object ID.
	 */
	using ObjectID = BcU32;

	/**
	 * Callback for actions.
	 */
	using ActionCallback = std::function< void( ObjectID ) >;
	
	/**
	 * Result from handle with interaction details.
	 */
	struct HandleResult 
	{
		bool WasClicked_ = false;
		BcU32 ButtonClicked_ = 0;
		BcU32 SelectedID_ = 0;
		MaVec3d WorldPosition_;
		MaVec2d ScreenPosition_;

		operator bool()
		{
			return WasClicked_;
		}
	};

	/**
	 * Draw obejct editor.
	 * Will handle any type represented by ReClass, will draw the default
	 * object editor. Will handle upcasting ReObject types.
	 */
	void ObjectEditor( DsImGuiFieldEditor* ThisFieldEditor, void* Data, const ReClass* Class, BcU32 Flags );

	/**
	 * Setup handle in world.
	 * @param ID ID to associate with handle.
	 * @param Name of handle. Included as part of the ID. Must be persistent until next frame.
	 * @param Position Position in world.
	 * @return Handle result.
	 */
	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d Position, BcF32 Size = 8.0f );

	/**
	 * Setup handle in world between two points.
	 * @param ID ID to associate with handle.
	 * @param Name of handle. Included as part of the ID. Use string literal, or permenant allocation.
	 * @param PointA Start position in world.
	 * @param PointB End position in world.
	 * @return Handle result.
	 */
	HandleResult Handle( BcU32 ID, const char* Name, const MaVec3d PointA, const MaVec3d PointB, BcF32 Size = 6.0f );

	/**
	 * Deselect currently selected handle.
	 */
	void DeselectHandle();

	/**
	 * Setup action.
	 * This will add (and perform) an action. If the ID & Name are persistent with the last one, then 
	 * the first @a Undo callback will be kept, and only @a Do will be executed.
	 * @param ID ID to associate with handle.
	 * @param Name of action. Included as part of the ID. Use string literal, or permenant allocation.
	 * @param ObjectID Object on which the action is being performed.
	 * @param Do Callback to perform action.
	 * @param Undo Callback to undo action.
	 * @param Commit Should commit after action.
	 */
	void Action( BcU32 ID, const char* Name, ObjectID Object, ActionCallback Do, ActionCallback Undo, bool Commit = true );

	/**
	 * Cancel action.
	 * This will execute the undo step of the currently active action.
	 */
	void CancelAction();

	/**
	 * Commit action.
	 * Will push the currently active action into the undo/redo stack.
	 */
	void CommitAction();

	/**
	 * Undo action.
	 */
	void UndoAction();

	/**
	 * Redo action.
	 */
	void RedoAction();

	/**
	 * Get an object ID for an object.
	 * @param Object Object we want an ID for.
	 * @param ID If non-zero, will replace existing object.
	 * @param ID.
	 */
	ObjectID GetObjectID( void* Object, ObjectID ID = 0 );

	/**
	 * Remove an object ID. Call when object has been removed.
	 * @param Object object to unregister.
	 * @param ID ID of object ot unregister. Can be zero.
	 */
	void RemoveObjectID( void* Object, ObjectID ID = 0 );

	/**
	 * Get an object.
	 * @param ID ID of object.
	 */
	void* GetObject( ObjectID ID );
	template< typename _Ty >
	_Ty* GetObjectByType( ObjectID ID )
	{
		return reinterpret_cast< _Ty* >( GetObject( ID ) );
	}
}
