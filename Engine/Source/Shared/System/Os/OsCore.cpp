/**************************************************************************
*
* File:		OsCore.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCore::OsCore()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCore::~OsCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCore::update()
{
	// Update input devices.
	for( auto InputDevice : InputDeviceList_ )
	{
		InputDevice->update();
	}

	// Update clients.
	for( auto Client : ClientList_ )
	{
		Client->update();
	}
}

//////////////////////////////////////////////////////////////////////////
// registerClient
void OsCore::registerClient( OsClient* pClient )
{
	ClientList_.push_back( pClient );
}

//////////////////////////////////////////////////////////////////////////
// unregisterClient
void OsCore::unregisterClient( OsClient* pClient )
{
	for( auto It( ClientList_.begin() ); It != ClientList_.end(); ++It )
	{
		if( (*It) == pClient )
		{
			ClientList_.erase( It );
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// registerInputDevice
void OsCore::registerInputDevice( OsInputDevice* InputDevice )
{
	InputDeviceList_.push_back( InputDevice );
}

//////////////////////////////////////////////////////////////////////////
// unregisterInputDevice
void OsCore::unregisterInputDevice( OsInputDevice* InputDevice )
{
	for( auto It( InputDeviceList_.begin() ); It != InputDeviceList_.end(); ++It )
	{
		if( (*It) == InputDevice )
		{
			InputDeviceList_.erase( It );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getNoofClients
size_t OsCore::getNoofClients() const
{
	return ClientList_.size();
}

//////////////////////////////////////////////////////////////////////////
// getNoofInputDevices
size_t OsCore::getNoofInputDevices() const
{
	return InputDeviceList_.size();
}

//////////////////////////////////////////////////////////////////////////
// getClient
OsClient* OsCore::getClient( size_t Index ) const
{
	if( Index < ClientList_.size() )
	{
		return ClientList_[ Index ];
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getInputDevice
OsInputDevice* OsCore::getInputDevice( size_t Index ) const
{
	if( Index < InputDeviceList_.size() )
	{
		return InputDeviceList_[ Index ];
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getInputDevice
OsInputDevice* OsCore::getInputDevice( BcName TypeName, BcU32 PlayerID ) const
{
	for( auto* InputDevice : InputDeviceList_ )
	{
		if( InputDevice->getTypeName() == TypeName && InputDevice->getPlayerID() == PlayerID )
		{
			return InputDevice;
		}
	}

	return nullptr;
}
