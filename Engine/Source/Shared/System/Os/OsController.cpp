#include "System/Os/OsController.h"
#include "System/Os/OsCore.h"
#include "System/Os/OsInputDevice.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
OsController::OsController()
{
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_UPDATE, this,
		[ this ]( EvtID, const EvtBaseEvent& )
		{
			BcAssert( Bindings_.size() == State_.size() );
			for( size_t Idx = 0; Idx < Bindings_.size(); ++Idx )
			{
				const auto& Binding = Bindings_[ Idx ];
				auto& State = State_[ Idx ];

				State.Prev_ = State.Curr_;

				if( Binding.Primary_.InputDeviceID_ != BcErrorCode )
				{
					auto InputDevice = InputDevices_[ Binding.Primary_.InputDeviceID_ ];
					State.Curr_ = InputDevice->getFloatInput( Binding.Primary_.InputID_ ) * Binding.Primary_.Multiplier_;
				}

				if( Binding.Secondary_.InputDeviceID_ != BcErrorCode )
				{
					auto InputDevice = InputDevices_[ Binding.Secondary_.InputDeviceID_ ];
					BcF32 Value = InputDevice->getFloatInput( Binding.Secondary_.InputID_ ) * Binding.Secondary_.Multiplier_;
					if( std::abs( Value ) > std::abs( State.Curr_ ) )
					{
						State.Curr_ = Value;
					}
				}
			}
			return evtRET_PASS;
		} );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsController::~OsController()
{
	OsCore::pImpl()->unsubscribeAll( this );
}

//////////////////////////////////////////////////////////////////////////
// addInputDevice
BcU32 OsController::addInputDevice( OsInputDevice* InputDevice )
{
	auto It = std::find( InputDevices_.begin(), InputDevices_.end(), InputDevice );
	if( It != InputDevices_.end() )
	{
		return static_cast< BcU32 >( It - InputDevices_.begin() );
	}
	InputDevices_.push_back( InputDevice );
	return static_cast< BcU32 >( InputDevices_.size() ) - 1;
}

//////////////////////////////////////////////////////////////////////////
// addPrimaryBinding
BcU32 OsController::addPrimaryBinding( BcName Name, BcU32 InputDeviceID, BcU32 InputID, BcF32 Multiplier )
{
	BcU32 ID = getBindingID( Name );
	if( ID == BcErrorCode )
	{
		Bindings_.emplace_back( Binding() );
		State_.resize( Bindings_.size() );
		ID = static_cast< BcU32 >( Bindings_.size() ) - 1;
	}

	auto& Binding = Bindings_[ ID ];
	Binding.Name_ = Name;
	Binding.Primary_.InputDeviceID_ = InputDeviceID;
	Binding.Primary_.InputID_ = InputID;
	Binding.Primary_.Multiplier_ = Multiplier;
	
	return ID;
}

//////////////////////////////////////////////////////////////////////////
// addSecondaryBinding
BcU32 OsController::addSecondaryBinding( BcName Name, BcU32 InputDeviceID, BcU32 InputID, BcF32 Multiplier )
{
	BcU32 ID = getBindingID( Name );
	if( ID == BcErrorCode )
	{
		Bindings_.emplace_back( Binding() );
		State_.resize( Bindings_.size() );
		ID = static_cast< BcU32 >( Bindings_.size() ) - 1;
	}

	auto& Binding = Bindings_[ ID ];
	Binding.Name_ = Name;
	Binding.Secondary_.InputDeviceID_ = InputDeviceID;
	Binding.Secondary_.InputID_ = InputID;
	Binding.Secondary_.Multiplier_ = Multiplier;
	
	return ID;
}

//////////////////////////////////////////////////////////////////////////
// getBindingID
BcU32 OsController::getBindingID( BcName Name ) const
{
	auto It = std::find_if( Bindings_.begin(), Bindings_.end(), 
		[ Name ]( const Binding& Other )
		{
			return Name == Other.Name_;
		} );
	if( It != Bindings_.end() )
	{
		return static_cast< BcU32 >( It - Bindings_.begin() );
	}
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// getState
BcF32 OsController::getState( BcU32 BindingID ) const
{
	if( BindingID < State_.size() )
	{
		return State_[ BindingID ].Curr_;
	}
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// isDown
bool OsController::isDown( BcU32 BindingID ) const
{
	if( BindingID < State_.size() )
	{
		return State_[ BindingID ].Curr_ > 0.5f;
	}
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// isUp
bool OsController::isUp( BcU32 BindingID ) const
{
	if( BindingID < State_.size() )
	{
		return State_[ BindingID ].Curr_ < 0.5f;
	}
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
// wasPressed
bool OsController::wasPressed( BcU32 BindingID ) const
{
	if( BindingID < State_.size() )
	{
		return State_[ BindingID ].Curr_ > 0.5f && State_[ BindingID ].Prev_ < 0.5f;
	}
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// wasReleased
bool OsController::wasReleased( BcU32 BindingID ) const
{
	if( BindingID < State_.size() )
	{
		return State_[ BindingID ].Curr_ < 0.5f && State_[ BindingID ].Prev_ > 0.5f;
	}
	return 0.0f;
}
